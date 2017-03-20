//nil. by Joseph Wilk <joseph@repl-electric.com>
uniform float iVolume;
uniform float iBeat;
uniform float iOffBeat;
uniform float iHat;
uniform float iGlobalBeatCount;
uniform sampler2D iChannel0;
uniform float     iExample;
uniform float     iMesh;
uniform float     iCellCount;
uniform float     iCellMotion;
uniform float     iSpaceMotion;
uniform float     iSpaceWeight;


uniform float iColorFactor;

const float pi = 3.14159265;
const mat2 m = mat2(0.80,  0.60, -0.60,  0.80);

vec3 hsv2rgb(float h, float s, float v) {
  return mix(vec3(1.), clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v;
}

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(2.9898,78.233))) * 58.5453);
}

float rand2(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hash3( vec2 p ){
  vec3 q = vec3(dot(p,vec2(127.1,311.7)),
              dot(p,vec2(269.5,183.3)),
              dot(p,vec2(419.2,371.9)) );
  return fract(sin(q)*43758.5453);
}

//Noise functions modified from originals created by Ian McEwan, Ashima Arts.
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
vec3 mod289_1_1(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289_1_1(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute_1_2(vec4 x) {
     return mod289_1_1(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt_1_3(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise_1_4(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D_1_5 = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g_1_6 = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g_1_6;
  vec3 i1 = min( g_1_6.xyz, l.zxy );
  vec3 i2 = max( g_1_6.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D_1_5.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289_1_1(i);
  vec4 p = permute_1_2( permute_1_2( permute_1_2(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D_1_5.wyz - D_1_5.xzx;

  vec4 j = p - 40 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1_1_7 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0_1_8 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1_1_7.xy,h.z);
  vec3 p3 = vec3(a1_1_7.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt_1_3(vec4(dot(p0_1_8,p0_1_8), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0_1_8 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  //x0 = x0*vec3(iBeat);

  float boom = texture2D(iChannel0, vec2(v.x,0.7)).x;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0_1_8,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }

vec3 backgroundColors(vec2 p, float speed) {
  vec3 col = vec3(0.0);
  float tone = iBeat;
  col += snoise_1_4(vec3(p * 0.1, iGlobalTime*speed*0.5)) * vec3(0.3, tone, 0.05);
  float boom = texture2D(iChannel0, col.xy).x;
  //boom = smoothstep(0.0,0.0, boom);
  col += snoise_1_4(vec3((iGlobalTime*0.00001)*boom, p * 0.4)) * vec3(0.1, tone, 0.4);
  return clamp(col, vec3(1.0), vec3(1.0));
}

float noise(float x, float y){return sin(1.5*x)*sin(1.5*y);}

mat2 mm2(in float a){
  float c = abs(cos(a));
  float s = sin(a);
  vec2 uv = gl_FragCoord.xy / iResolution.x;
  float FREQ_SCALE = (4096.0/4096.0);
  float AMP_SCALE = 0.5;
  float fi = FREQ_SCALE*uv.y;
  float fid  = FREQ_SCALE/4096.0/2.0;
  float adjust = AMP_SCALE * 0.5 *   (max(0.0, texture2D(iChannel0, vec2(fi,0.25)).x) +
                                      max(0.0, texture2D(iChannel0, vec2(fi+fid,0.25)).x));

  if(iVolume == 0.0){
    adjust = 1;
  }

  return mat2(c * adjust, -s * adjust,s * adjust ,c * adjust);
}

float saturate(float a){ return clamp( a, 0.0, 1.0 );}
// Fractional Brownian Motion code by IQ.
// http://en.wikipedia.org/wiki/Fractional_Brownian_motion

const float linesmooth = 0.0333;
const float tau = 6.28318530717958647692;

vec3 hsvToRgb(float mixRate, float colorStrength){
  float colorChangeRate = 18.0;
  float time = fract(iGlobalTime/colorChangeRate);
  float movementStart = (mod(iGlobalBeatCount,16) == 0) ? 1.0 : 0.5;
  vec3 x = abs(fract((mod(iGlobalBeatCount,16)-1+time) + vec3(2.,3.,1.)/3.) * 6.-3.) - 1.;
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

float smoothbump(float center, float width, float x){
  float w2 = width/2.0;
  float cp = center+w2;
  float cm = center-w2;
  float c = smoothstep(cm, center, x) * (1.0-smoothstep(center, cp, x));
  return c;
}

float makePoint(float x,float y,float fx,float fy,float sx,float sy,float t){
   float bump = iBeat*0.2;
   //sy+=bump;

   float cellSize = 0.5+iBeat*0.2;
   cellSize = cellSize-iOffBeat*0.1;

   cellSize -= max(iHat,0.9);

   float motionFactor = iCellMotion;
   t = t*(motionFactor);
   float xx=x+sin(t*fx)*sx;
   float yy=y+tan(t*fy)*sy;
   xx += bump*0.025;
   yy += bump*0.025;
   vec2 uv = gl_FragCoord.xy / iResolution.x;
   float sound = texture2D(iChannel0, vec2(uv.x,.15)).x;

   float d = 0.0;
   d = sqrt(xx*xx+yy*yy)+sound*0.05;
   //      if(iOffBeat == 1.0){
   //       d *= dot(xx+yy,xx+yy)*10.0;
   d -= dot(xx+yy*0.0001,xx+yy*0.0001)*0.5;
   //      }

   return cellSize/d;
}

vec4 dropping(void) {
   vec2 p=(gl_FragCoord.xy/iResolution.x)*2.0-vec2(1.0,iResolution.y/iResolution.x);
   float time = iGlobalTime * 0.2;
   float cells = iCellCount;

   p=p*2.5;

   float x=p.x;
   float y=p.y;

   float a= 0.0;
   if(cells > 1){
   a=a+makePoint(x,y,3.3,2.9,0.3,0.3,time);
   }
   if(cells > 2){
   a=a+makePoint(x,y,1.9,2.0,0.4,0.4,time);
   }
   if(cells > 3){
   a=a+makePoint(x,y,0.8,0.7,0.4,0.5,time);
   }
   if(cells > 4){
   a=a+makePoint(x,y,2.3,0.1,0.6,0.3,time);
   }
   if(cells > 5){
   a=a+makePoint(x,y,0.8,1.7,0.5,0.4,time);
   }
   if(cells > 6){
   a=a+makePoint(x,y,0.3,1.0,0.4,0.4,time);
   }
   if(cells > 7){
   a=a+makePoint(x,y,1.4,1.7,0.4,0.5,time);
   }
   if(cells > 8){
   a=a+makePoint(x,y,1.3,2.1,0.6,0.3,time);
   }
   if(cells > 9){
   a=a+makePoint(x,y,1.8,1.7,0.5,0.4,time);
   }

   float b=0.0;
   if(cells > 10){
   b=b+makePoint(x,y,1.2,1.9,0.3,0.3,time);
   }
   if(cells > 11){
   b=b+makePoint(x,y,0.7,2.7,0.4,0.4,time);
   }
   if(cells > 12){
   b=b+makePoint(x,y,1.4,0.6,0.4,0.5,time);
   }
   if(cells > 13){
   b=b+makePoint(x,y,2.6,0.4,0.6,0.3,time);
   }
   if(cells > 14){
   b=b+makePoint(x,y,0.7,1.4,0.5,0.4,time);
   }
   if(cells > 15){
   b=b+makePoint(x,y,0.7,1.7,0.4,0.4,time);
   }
   if(cells > 16){
   b=b+makePoint(x,y,0.8,0.5,0.4,0.5,time);
   }
   if(cells > 17){
   b=b+makePoint(x,y,1.4,0.9,0.6,0.3,time);
   }
   if(cells > 18){
   b=b+makePoint(x,y,0.7,1.3,0.5,0.4,time);
   }

   float c=0.0;
   c=c+makePoint(x,y,3.7,0.3,0.3,0.8,time);
   if(cells > 1){
   c=c+makePoint(x,y,1.9,1.3,0.4,0.4,time);
   }
   if(cells > 2){
   c=c+makePoint(x,y,0.8,0.9,0.4,0.5,time);
   }
   if(cells > 3){
   c=c+makePoint(x,y,1.2,1.7,0.6,0.3,time);
   }
   if(cells > 4){
   c=c+makePoint(x,y,0.3,0.6,0.5,0.4,time);
   }
   if(cells > 5){
   c=c+makePoint(x,y,0.3,0.3,0.4,0.4,time);
   }
   if(cells > 6){
   c=c+makePoint(x,y,1.4,0.8,0.4,0.5,time);
   }
   if(cells > 7){
   c=c+makePoint(x,y,0.2,0.6,0.6,0.3,time);
   }
   if(cells > 8){
   c=c+makePoint(x,y,1.3,0.5,0.5,0.4,time);
   }

   //c*=1.0;
   //a*=1.0;
   //c*=1.0;

   vec3 d=vec3(0.0);
   //   vec3 aa = vec3(a*0.01,a*0.01,a*0.2+iBeat*0.1);
   //   vec3 bb = vec3(b*0.2,b*0.1,b*0.01);
   //vec3 cc = vec3(c*1.0,c*1.0,c*1.0);

   float speed = iGlobalTime*0.01;
   vec2 px = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;
   vec3 color = backgroundColors(px, speed*0.001);

   d = c*0.1+ color;

   float circleScale = 0.1;
   return vec4(d.x-1.0*circleScale,d.y-1.3,d.z-1.1,1.0);
}

const int MAGIC_BOX_ITERS = 9;
const float MAGIC_BOX_MAGIC = 0.76;

float sum( vec2 a ) { return a.x + a.y; }

float magicBox(vec3 p){
    p = 1.0 - abs(1.0 - mod(p, 2.0));

    float lastLength = length(p);
    float tot = 0.0;
    float movement = 0.0;

    movement = (-iBeat) + (iOffBeat);
    movement = iGlobalTime*0.0001;
    vec2 uv = gl_FragCoord.xy / iResolution.yy;

    p.x += iBeat*0.1;
    p.y += iOffBeat*0.1;

    for (int i=0; i < MAGIC_BOX_ITERS; i++) {
      p = abs(p)/(lastLength*lastLength) - MAGIC_BOX_MAGIC;
      float newLength = length(p);
      tot += abs(newLength-lastLength);
      lastLength = newLength;
    }

    return tot;
}

const mat3 M = mat3(0.28862355854826727, 0.6997227302779844, 0.6535170557707412,
                    0.06997493955670424, 0.6653237235314099, -0.7432683571499161,
                    -0.9548821651308448, 0.26025457467376617, 0.14306504491456504);

vec4 starlight(void) {
  vec2 modp = mod( floor( gl_FragCoord.xy ), 2. );
  float m = mod( sum( modp ), 2. ) * 1.5 - .5;
  vec2 uv = gl_FragCoord.xy / iResolution.yy;

  if(rand2(uv) > 0.5){
    uv.y += iGlobalTime * 0.9 * iSpaceMotion;
  }
  else{
    uv.y -= iGlobalTime * 0.9 * iSpaceMotion;
  }

  vec3 p = 0.5*M*vec3(uv, 0.0);

  float result = magicBox(p);
  result *= 0.1;
  //result = clamp( result, 0., 1. );
    if ( gl_FragCoord.x < iResolution.x / 2. ) {
      return vec4(vec3(min( result, fwidth( m * result ) / 3. )),1.0);
    } else {
      return vec4(vec3(result),1.0);
    }
}

void main(void){
  vec2 uv = gl_FragCoord.xy / iResolution.x;
  //vec4 c = vec4(1.);
   vec4 c = vec4(0.9);

  

  float speed = 0.09;
  vec2 p = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;
  vec3 color = backgroundColors(p, speed);

  if(iMesh <= 0.0){
    c+= dropping();
    vec4 spWeight = vec4(0.0);
    if(iSpaceWeight > 0.0){
      c+= starlight()*iSpaceWeight;
    }
  }
  else{
    c = mix(c,vec4(color,0.0),0.9);
  }
  gl_FragColor = lineDistort(c,uv);
}
