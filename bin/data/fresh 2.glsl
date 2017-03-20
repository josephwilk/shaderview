uniform float iBeat;
// a study on raymarching, soft-shadows, ao, etc
// borrowed heavy from others, esp @cabbibo and @iquilezles and more
// by @eddietree

#define float3 vec3

const float INTERSECTION_PRECISION = 0.0001;
const int NUM_OF_TRACE_STEPS = 2;

float distSphere(vec3 p, float radius) 
{
    return length(p) - radius;
}

// by shane : https://www.shadertoy.com/view/4lSXzh
float Voronesque( in vec3 p )
{
    vec3 i  = floor(p + dot(p, vec3(0.333333)) );  p -= i - dot(i, vec3(0.166666)) ;
    vec3 i1 = step(0., p-p.yzx), i2 = max(i1, 1.0-i1.zxy); i1 = min(i1, 1.0-i1.zxy);    
    vec3 p1 = p - i1 + 0.166666, p2 = p - i2 + 0.333333, p3 = p - 0.5;
    vec3 rnd = vec3(7, 157, 113); // I use this combination to pay homage to Shadertoy.com. :)
    vec4 v = max(0.5 - vec4(dot(p, p), dot(p1, p1), dot(p2, p2), dot(p3, p3)), 0.);
    vec4 d = vec4( dot(i, rnd), dot(i + i1, rnd), dot(i + i2, rnd), dot(i + 1., rnd) ); 
    d = fract(sin(d)*262144.)*v*2.; 
    v.x = max(d.x, d.y), v.y = max(d.z, d.w), v.z = max(min(d.x, d.y), min(d.z, d.w)), v.w = min(v.x, v.y); 
    return  max(v.x, v.y) - max(v.z, v.w); // Maximum minus second order, for that beveled Voronoi look. Range [0, 1].
    
}

mat3 calcLookAtMatrix( in vec3 ro, in float3 ta, in float roll )
{
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(sin(roll),cos(roll),0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
    return mat3( uu, vv, ww );
}

void doCamera( out vec3 camPos, out vec3 camTar, in float time, in vec2 mouse )
{
    float radius = 7.0;
    float theta = 0.3 + 5.0*mouse.x;// - iGlobalTime*0.3;
    float phi = 3.14159*0.4;//5.0*mouse.y;
    
    float pos_x = radius * cos(theta) * sin(phi);
    float pos_z = radius * sin(theta) * sin(phi);
    float pos_y = radius * cos(phi);
    
    camPos = vec3(pos_x, pos_y, pos_z);
    camTar = vec3(0.0,0.0,0.0);
}

float smin( float a, float b, float k )
{
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}

float opS( float d1, float d2 )
{
    return max(-d1,d2);
}

float opU( float d1, float d2 )
{
    return min(d1,d2);
}

// noise func
float hash( float n ) { return fract(sin(n)*753.5453123); }
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}

// checks to see which intersection is closer
// and makes the y of the vec2 be the proper id
vec2 opU( vec2 d1, vec2 d2 ){
	return (d1.x<d2.x) ? d1 : d2; 
}

float opI( float d1, float d2 )
{
    return max(d1,d2);
}

//--------------------------------
// Modelling 
//--------------------------------
vec2 map( vec3 pos )
{  
    //float t1 = sphere;
    float t1 = Voronesque(pos);// - (sin(pos.x*10.0)*0.5+0.5);
    
    float sphere = distSphere(pos, 1.0 + t1*0.75 + sin(iGlobalTime*iBeat+pos.x*1.));// + noise(pos * 1.0 + iGlobalTime*0.75);   
    //t1 = max( t1, sphere );
    t1 = sphere;
   
   	return vec2( t1, 1.0 );
}

float shadow( in vec3 ro, in vec3 rd )
{
    const float k = 2.0;
    
    const int maxSteps = 1;
    float t = 0.0;
    float res = 1.0;
    
    for(int i = 0; i < maxSteps; ++i) {
        
        float d = map(ro + rd*t).x;
            
        if(d < INTERSECTION_PRECISION) {
            
            return 0.0;
        }
        
        res = min( res, k*d/t );
        t += d;
    }
    
    return res;
}


float ambientOcclusion( in vec3 ro, in vec3 rd )
{
    const int maxSteps = 1;
    const float stepSize = 0.05;
    
    float t = 0.0;
    float res = 0.0;
    
    // starting d
    float d0 = map(ro).x;
    
    for(int i = 0; i < maxSteps; ++i) {
        
        float d = map(ro + rd*t).x;
		float diff = max(d-d0, 0.0);
        
        res += diff;
        
        t += stepSize;
    }
    
    return res;
}

vec3 calcNormal( in vec3 pos ){
    
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}

bool renderRayMarch(vec3 rayOrigin, vec3 rayDirection, inout vec3 color ) {
    const int maxSteps = NUM_OF_TRACE_STEPS;
        
    float t = 0.0;
    float d = 0.0;
    
    vec3 lightDir = normalize(vec3(1.0,0.4,0.0));
    
    for(int i = 0; i < maxSteps; ++i) 
    {
        vec3 currPos = rayOrigin + rayDirection * t;
        d = map(currPos).x;
        if(d < INTERSECTION_PRECISION) {
            
            break;
        }
        
        t += d;
    }
    
    if(d < INTERSECTION_PRECISION) 
    {    
        vec3 currPos = rayOrigin + rayDirection * t;
        vec3 normal = calcNormal( currPos );
        vec3 normal_distorted = calcNormal( currPos +  noise(currPos*1.5 + vec3(0.0,0.0,sin(iGlobalTime*iBeat))) );
        float shadowVal = shadow( currPos - rayDirection* 0.01, lightDir  );
        float ao = ambientOcclusion( currPos - normal*0.01, normal );

        float ndotl = abs(dot( -rayDirection, normal ));
        float ndotl_distorted = abs(dot( -rayDirection, normal_distorted ));
        float rim = pow(1.0-ndotl, 6.0);
        float rim_distorted = pow(1.0-ndotl_distorted, 6.0);

        //color = vec3(0.9);
        color = mix( color, normal*0.5+vec3(0.5), rim_distorted+0.1 );

        //color = normal;

        //color = normal;
        //color *= vec3(mix(0.25,1.0,shadowVal));
        //color *= vec3(mix(0.8,1.0,ao));
        color += rim;

        return true;
    }
    
    return false;
}


void main( void )
{
	vec2 p = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;
    vec2 m = iMouse.xy/iResolution.xy;
    
    // camera movement
    vec3 ro, ta;
    doCamera( ro, ta, iGlobalTime*iBeat, m );

    // camera matrix
    mat3 camMat = calcLookAtMatrix( ro, ta, 0.0 );  // 0.0 is the camera roll
    
	// create view ray
	vec3 rd = normalize( camMat * vec3(p.xy,2.0) ); // 2.0 is the lens length
    
    // calc color
    vec3 col = vec3(0.9);
    renderRayMarch( ro, rd, col );
    
    // vignette, OF COURSE
    //float vignette = 1.0-smoothstep(1.0,2.5, length(p));
    //col.xyz *= mix( 0.7, 1.0, vignette);
        
    gl_FragColor = vec4( col, 1. );
}