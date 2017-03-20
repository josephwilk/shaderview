/*
	Box Divide
	2015 BeyondTheStatic

	As usual, my code is free to use however you wish :)
*/
uniform sampler2D iChannel0;
const int MaxIters	= 10; // maximum number of iterations
const float RndAmt	= .5; // amount of random influence (0.0-1.0)

// ubiquitous rand() function
float rand(vec2 p){ return fract(sin(dot(p, vec2(12.9898, 78.233)))*43758.5453); }

vec4 lineDistort(vec4 cTextureScreen, vec2 uv1){
  float sCount = 900.;
  float nIntensity=0.1;
  float sIntensity=0.2;
  float noiseEntry = 0.0;
  float accelerator= 1000.0;

  // sample the source
  float x = uv1.x * uv1.y * iGlobalTime * accelerator;
  x = mod( x, 13.0 ) * mod( x, 123.0 );
  float dx = mod( x, 0.05 );
  vec3 cResult = cTextureScreen.rgb + cTextureScreen.rgb * clamp( 0.1 + dx * 100.0, 0.0, 1.0 );
  // get us a sine and cosine
  vec2 sc = vec2( sin( uv1.y * sCount ), cos( uv1.y * sCount ) );
  // add scanlines
  cResult += cTextureScreen.rgb * vec3( sc.x, sc.y, sc.x ) * sIntensity;

  // interpolate between source and result by intensity
  cResult = cTextureScreen.rgb + clamp(nIntensity, noiseEntry,1.0 ) * (cResult - cTextureScreen.rgb);

  return vec4(cResult, cTextureScreen.a);
}

// this function returns a vec2 which can be used for a texture or whatever
// p: input UVs
// l: x & y size of initial box (be aware of aspect)
vec2 boxDivide(in vec2 p, in vec2 l) {
    // initial random values
    vec2 r = mix(vec2(.5), vec2(rand(vec2(.02)), rand(vec2(.03))), RndAmt);
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    // split again & again
    float speed = 20.0;
    for(int i=0; i<MaxIters; i++) {
        // animate (remove to always use maximum iterations)
          float sound = texture2D(iChannel0, vec2(uv.x,.15)).x; if(i>int(float(MaxIters)*(.8+.5*cos(iGlobalTime/float(MaxIters)*speed+3.141592)))-1) break;
        
        // vertical split
        if(l.x<l.y) {
            if(p.x<r.x) {
                l.x /= r.x;
                p.x = p.x/r.x;
                r = mix(vec2(0.5), fract(r+rand(vec2(r.x, 0.))), RndAmt);
            }
            else {
                l.x /= (1.-r.x);
                p.x = (p.x-r.x)/(1.-r.x);
                r = mix(vec2(0.5), fract(r+rand(vec2(-r.x, .2))), RndAmt);
            }
        }
        // horizontal split
        else {
            if(p.y<r.y) {
                l.y /= r.y;
                p.y = p.y/r.y;
                r = mix(vec2(0.5), fract(r+rand(vec2(0., r.y))), RndAmt);
            }
            else {
                l.y /= (1.-r.y);
                p.y = (p.y-r.y)/(1.-r.y);
                r = mix(vec2(.5), fract(r+rand(vec2(.2, -r.y))), RndAmt);
            }
        }
    }
    return p;
}

void main(void )
{
	vec2 res = iResolution.xy;
    vec2 uv = gl_FragCoord.xy / res;
    
    // calling the function
    vec2 p = boxDivide(uv, vec2(res.y/res.x, 1.));
    
    // making a basic rounded box
    vec2 p2 = p - .5;
    float f = 1.-16.*(p2.x*p2.x*p2.x*p2.x+p2.y*p2.y*p2.y*p2.y);
    
    gl_FragColor = lineDistort(vec4(p*f*1.5, f, 1.),uv);
}