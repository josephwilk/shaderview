uniform float iBeat;
uniform float iIt;
uniform float iColor;
uniform float iZoom;
uniform sampler2D iChannel0;

vec3 hsvToRgb(float mixRate, float colorStrength){
  float colorChangeRate = 18.0;
  float time = fract(iGlobalTime*0.05/colorChangeRate);
  float movementStart = (mod(iGlobalTime*0.05,16) == 0) ? 1.0 : 0.5;
  vec3 x = abs(fract((mod(iGlobalTime*0.05,16)-1+time) + vec3(2.,3.,1.)/3.) * 6.-3.) - 1.;
  vec3 c = clamp(x, 0.,1.);
  //c = c*iBeat;
  //c = c * clamp(iBeat, 0.1, 0.4)+0.6;
  return mix(vec3(1.0), c, mixRate) * colorStrength;
}

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

// by Nikos Papadopoulos, 4rknova / 2015
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
vec4 fractal(vec2 uv){
  float time = iGlobalTime*.8;
  float k = cos(time*0.01);
  float l = sin(time*0.01*iZoom)*iZoom;

  float s = .2;
  float rot = 0.99;
  for(int i=0; i<30; ++i) {
      float sound = texture2D(iChannel0, vec2(uv.x,.0)).x;
      uv -= sound*0.005;
      uv  = abs(uv) - s;        // Mirror
      uv *= mat2(k,-l,l*rot,k); // Rotate
      s  *= .95156;            // Scale
  }

  float x = .5 + .5*cos(10.28318*(length(uv)));
  vec3 color = hsvToRgb(0.5,1.0)*iColor;
  vec4 r = vec4(vec3(x)*color,1.0);
  r.x *= 0.1;
  r.y *= 0.1;
  r.z *= 0.1;
  return r;
}

void main(void){
  vec2 uv = 0.275 * gl_FragCoord.xy / iResolution.y;
  vec4 r = fractal(uv);
  gl_FragColor = lineDistort(r, uv);
}
