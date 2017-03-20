//Raymarch settings
uniform sampler2D iChannel0;
#define MIN_DIST 0.001
#define MAX_DIST 12.0
#define MAX_STEPS 25
#define STEP_MULT 0.5
#define NORMAL_OFFS 0.002

//Scene settings

#define QUADS_PER_UNIT 8.0
#define HAZE_COLOR vec3(0.15, 0.00, 0.10)
#define GRID_LINE_RADIUS 2.0
//#define SHOW_RAY_COST

//Derived settings
#define QUAD_SIZE (1.0/QUADS_PER_UNIT)

float pi = atan(1.0) * 4.0;
float tau = atan(1.0) * 8.0;

struct MarchResult
{
    vec3 position;
    vec3 normal;
    float dist;
    float steps;
};

//Returns the height at a given position.
float Height(vec2 p)
{
    p *= QUAD_SIZE;
    
    float h = texture2D(iChannel0, p * 0.1 + iGlobalTime * 0.01, -99.0).x * 0.5;
    
    h += sin(length(p) * 2.0 + iGlobalTime) * 0.25;
    
	return h;
}

//Returns a rotation matrix for the given angles around the X,Y,Z axes.
mat3 Rotate(vec3 angles)
{
    vec3 c = cos(angles);
    vec3 s = sin(angles);
    
    mat3 rotX = mat3( 1.0, 0.0, 0.0, 0.0,c.x,s.x, 0.0,-s.x, c.x);
    mat3 rotY = mat3( c.y, 0.0,-s.y, 0.0,1.0,0.0, s.y, 0.0, c.y);
    mat3 rotZ = mat3( c.z, s.z, 0.0,-s.z,c.z,0.0, 0.0, 0.0, 1.0);

    return rotX * rotY * rotZ;
}

//==== Distance field operators/functions by iq. ====
float opU( float d1, float d2 )
{
    return min(d1,d2);
}

float opS( float d1, float d2 )
{
    return max(-d1, d2);
}

float sdSphere( vec3 p, float s )
{
  return length(p) - s;
}

//Modified to create a plane from 3 points.
float sdPlane( vec3 p, vec3 p0, vec3 p1, vec3 p2 )
{
  return dot(p - p0, normalize(cross(p0 - p1, p0 - p2)));
}
//===================================================

/*
Distance to a vertical quad comprised of two triangles.

1-----2
| 0  /|
|  /  |
|/  1 |
0-----3
*/
float sdVQuad( vec3 p, float h0, float h1, float h2, float h3)
{
    float s = QUAD_SIZE;
       
    float diag = sdPlane(p, vec3(0, 0, 0),vec3(s, s, 0),vec3(0, 0, s));
    
    float tri0 = sdPlane(p, vec3(0, 0,-h0),vec3(0, s,-h1),vec3(s, s,-h2)); //Triangle 0 (0,1,2)
    tri0 = opS(-diag, tri0);
    
    float tri1 = sdPlane(p, vec3(0, 0,-h0),vec3(s, s,-h2),vec3(s, 0,-h3)); //Triangle 1 (0,2,3)
    tri1 = opS(diag, tri1);
    
    float d = min(tri0,tri1);
    
    return d;
}

//Distance to the scene
float Scene(vec3 p)
{
    float d = MAX_DIST;
    
    vec3 pm = vec3(mod(p.xy, vec2(QUAD_SIZE)), p.z);
    
    vec2 uv = floor(p.xy / QUAD_SIZE);
    
    float v0 = Height(uv + vec2(0, 0));
    float v1 = Height(uv + vec2(0, 1));
    float v2 = Height(uv + vec2(1, 1));
    float v3 = Height(uv + vec2(1, 0));
    
    d = sdVQuad(pm - vec3(0.0 ,0.0, 0.0), v0, v1, v2, v3);
    
    d = opU(d, -sdSphere(p, MAX_DIST - 1.0));
    
	return d;
}

//Surface normal at the current position
vec3 Normal(vec3 p)
{
    vec3 off = vec3(NORMAL_OFFS, 0, 0);
    return normalize
    ( 
        vec3
        (
            Scene(p + off.xyz) - Scene(p - off.xyz),
            Scene(p + off.zxy) - Scene(p - off.zxy),
            Scene(p + off.yzx) - Scene(p - off.yzx)
        )
    );
}

//Raymarch the scene with the given ray
MarchResult MarchRay(vec3 orig,vec3 dir)
{
    float steps = 0.0;
    float dist = 0.0;
    
    for(int i = 0;i < MAX_STEPS;i++)
    {
        float sceneDist = Scene(orig + dir * dist);
        
        dist += sceneDist * STEP_MULT;
        
        steps++;
        
        if(abs(sceneDist) < MIN_DIST)
        {
            break;
        }
    }
    
    MarchResult result;
    
    result.position = orig + dir * dist;
    result.normal = Normal(result.position);
    result.dist = dist;
    result.steps = steps;
    
    return result;
}

//Scene texturing/shading
vec3 Shade(MarchResult hit, vec3 direction, vec3 camera)
{
    vec3 color = vec3(1.0);
    
    //Triangle grid pattern
    vec2 gridRep = mod(hit.position.xy, vec2(QUAD_SIZE)) / float(QUAD_SIZE) - 0.5;
    
    float grid = 0.5 - max(abs(gridRep.x), abs(gridRep.y));
    grid = min(grid, abs(dot(gridRep.xy, normalize(vec2(-1, 1)))));
    
    float lineSize = GRID_LINE_RADIUS * hit.dist / iResolution.y / float(QUAD_SIZE);
    
    color *= 1.0-smoothstep(lineSize, lineSize * 0.25, grid);
    color = color * 0.75 + 0.25;
    
    //Lighting
    float ambient = 0.1;
    float diffuse = 0.5 * -dot(hit.normal, direction);
    float specular = 1.1 * max(0.0, -dot(direction, reflect(direction, hit.normal)));
    
    color *= vec3(ambient + diffuse + pow(specular, 5.0));
	
    //Fog / haze
    float sky = smoothstep(MAX_DIST - 1.0, 0.0, length(hit.position));
    float haze = 1.0 - (hit.steps / float(MAX_STEPS));
    
    vec3 skycol = mix(HAZE_COLOR, vec3(0), clamp(-hit.position.z * 0.2, 0.0, 1.0));
    
    color = mix(skycol, color, sky * haze);
    
    return color;
}

void main( void )
{
    vec2 res = iResolution.xy / iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.y;
    
    //Camera stuff   
    vec3 angles = vec3(0);
    
    if(iMouse.xy == vec2(0,0))
    {
        angles.y = tau * (1.5 / 8.0);
        angles.x = iGlobalTime * 0.1;
    }
    else
    {    
    	angles = vec3((iMouse.xy / iResolution.xy) * pi, 0);
        angles.xy *= vec2(2.0, 1.0);
    }
    
    angles.y = clamp(0.0, tau / 4.0, angles.y);
    
    mat3 rotate = Rotate(angles.yzx);
    
    vec3 orig = vec3(0, 0,-2) * rotate;
    orig -= vec3(0, 0, 1);
    
    vec3 dir = normalize(vec3(uv - res / 2.0, 0.5)) * rotate;
    
    //Ray marching
    MarchResult hit = MarchRay(orig, dir);
    
    //Shading
    vec3 color = Shade(hit, dir, orig);
    
    #ifdef SHOW_RAY_COST
    color = mix(vec3(0,1,0), vec3(1,0,0), hit.steps / float(MAX_STEPS));
    #endif
    
	gl_FragColor = vec4(color, 1.0);
}