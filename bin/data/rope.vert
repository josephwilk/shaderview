uniform float iCircle;
uniform float iBeat;
uniform float iVertexCount;
uniform sampler2D iChannel0;
uniform float iLineMode;
uniform float iX;
uniform float iY;
uniform float iP;

uniform float iIntro;


uniform float iCrazy;
uniform float iSpeed;

uniform float iBits;
uniform float iMotion;

uniform float iPointSize;
uniform float iDistort;
uniform float iForm;
uniform float iSize;
uniform float iHorse;
uniform float iDir;
uniform float iFuzz;


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hsv2rgb(vec3 c) {
  c = vec3(c.x, clamp(c.yz, 1.0, 1.0));
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

#define NUM_SEGMENTS 21.0
#define NUM_POINTS (NUM_SEGMENTS * 3.0)
#define STEP 8.0
#define PI radians(180.)

void swirl(){
  float localTime = iGlobalTime*iMotion + 60.0;
  float point = mod(floor(gl_VertexID / 8.0) + mod(gl_VertexID, 2.0) * STEP, NUM_SEGMENTS);
  float count = floor(gl_VertexID / NUM_POINTS);
  float offset = count * 0.02;
  float angle = point * PI * 3.14 / NUM_SEGMENTS + offset;
  float radius = 0.1;
  float c = cos(angle + localTime) * radius;
  float s = sin(angle + localTime) * radius;
  float orbitAngle = count * 0.004;
  float oC = cos(orbitAngle + localTime * count * 0.01) * sin(orbitAngle);
  float oS = sin(orbitAngle + localTime * count * 0.01) * sin(orbitAngle);

  #ifdef FIT_VERTICAL
    vec2 aspect = vec2(iResolution.y / iResolution.x, 1);
  #else
    vec2 aspect = vec2(1, iResolution.x / iResolution.y);
  #endif

  vec2 xy = vec2(
      oC + c,
      oS + s);
  gl_Position = vec4(xy * aspect + iMouse * 0.1, 0, 1);

  float hue = (localTime * 0.01 + count * 1.001);
  gl_PointSize = (sin(iGlobalTime*iMotion)*0.5+0.5)*4;
  gl_FrontColor = vec4(hsv2rgb(vec3(hue, 1, 1)), 1);
}



float rand2(vec2 co){
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 posf2(float t, float i) {

  //t = 0.0*t;
  //i = 0.0001;
  //i /= 0.1;
	return vec3(
      sin(t+i*.9553) +
      sin(t*1.311+i) +
      sin(t*1.4+i*1.53) +
      sin(t*1.84+i*.76),
      sin(t+i*.79553+2.1) +
      sin(t*1.311+i*1.1311+2.1) +
      sin(t*1.4+i*1.353-2.1) +
      sin(t*1.84+i*.476-2.1),
      sin(t+i*.5553-2.1) +
      sin(t*1.311+i*1.1-2.1) +
      sin(t*1.4+i*1.23+2.1) +
      sin(t*1.84+i*.36+2.1)
	) * 0.2;
}

vec3 posf0(float t) {
  return posf2(t,-1.)*3.5;
}

vec3 posf(float t, float i) {
//  float snd = texture2D(iChannel0, vec2(0.75, i)).x*0.1;
  //i -= (iHorse+1.0)*0.5;
  //t *= (iHorse+1.0)*0.5;
    return posf2(t*.3,i) + posf0(t);
}

vec3 push(float t, float i, vec3 ofs, float lerpEnd) {
  float snd = pow(texture2D(iChannel0, vec2(.02+.5 * ofs.x, 0.)).x, 8.);
  float wave = texture2D(iChannel0, vec2(ofs.x,0.75)).x *0.8;
  vec3 pos = posf(t,i) + ofs;

  //pos.x += snd2;
  //pos += iBeat*0.1;

  vec3 posf = fract(pos+.5)-.5;
  float l = length(posf)*2.;
  vec3 r = (-posf + posf/l) * (1-smoothstep(lerpEnd,1.,l));

  //Lines through jumping
  //return 0.01*rand2(vec2(t,i))+r;

  if(iDistort > 0.0){
    if(rand2(vec2(i,i)) > 0.5){
      r.x += min(iDistort*100,199)*rand2(vec2(i,i));
    }
    if(rand2(vec2(i,i)) < 0.2){
      r.y += min(iDistort*100,0.2)*rand2(vec2(i,i));
    }
    if(rand2(vec2(i,i)) > 0.9){
      r.z -= min(iDistort*2,0.2)*rand2(vec2(i,i));
    }
  }
  return r*iFuzz;
}

void bits() {
  float time = iGlobalTime * iMotion;

  if(iHorse < 0){
    //time = -time;
  }

  time += iBeat*0.01;
  float snd = pow(texture2D(iChannel0, vec2(gl_VertexID, 0.)).x, 8.);
  float constrict = min(iDistort+snd*0.1,2.0);
  float i = (gl_VertexID) * constrict;

  vec3 pos = posf(time,i);
  vec3 ofs = vec3(snd);

  for (float f = -10.1; f < 10.; f++) {
    snd = texture2D(iChannel0, vec2(f * ofs.x, 0.8)).x*0.0005;
    if(rand2(ofs.xy) > 0.5){
      ofs += push((time+snd+f*.05)*0.1,i,ofs, 199.-exp(-f*.1));
    }
    else{
      //ofs += push((time-snd+f*.05),i,ofs, 2.-exp(-f*.1));
      ofs *= push((time-snd+f*.05),i,ofs, 2.-exp(-f*.1));
    }
  }

  float formWeight = iForm;
  if(formWeight > 0.0){
    ofs += push(time, i, ofs, .999) * formWeight;
  }

  pos -= posf0(time);
  pos += ofs;

  //pos.yz *= mat2(.8,9.6,-.6,.8);
  pos.yz   *= mat2(.8, 0.6,-.6,.8);
  pos.xz   *= mat2(.8,.6,-.6,.8);

  if(iSize > 0.0){
    pos.z *= iSize;
  }

  pos *= 1.;
  pos.z /= 6.0;
  pos.xy *= 0.6/pos.z;

  gl_Position = vec4(pos.x, pos.y*iResolution.x/iResolution.y, 0, 1);
  float size = (1./pos.z)*6;
  if(iPointSize < 0.1){
    if(mod(iGlobalTime, 128.0) > 64.0){
      size = ((1./pos.z)*iPointSize)+iBeat;
    }
  }
  else{
    size = ((1./pos.z)*iPointSize)+iBeat*5;
  }
  gl_PointSize  = max(size, 0.000000001);
  gl_FrontColor = vec4(abs(normalize(ofs))*.3+.7,1);
}

void main() {
  float time = iGlobalTime*(0.015+ iCrazy);
  float vertexCount = max(iVertexCount,1);
  vertexCount = min(vertexCount, max(iCircle,1));

  if(iIntro == 3.0){
    vertexCount = 100000;
  }

  float vertexId = gl_VertexID;
  float aspect = iResolution.x / iResolution.y;


  float cTmp = 64.0;
  if(iP > 0.0){
    cTmp = iP;
  }
  if(iIntro == 3.0){
    cTmp = 64;
  }

  float cPoints = cTmp * 1.0;//iCircle; //64
  float circles = ceil(vertexCount / cPoints);
  float cId = floor(vertexId / cPoints);
  float cNorm = cId / circles;
  float vId = mod(vertexId, cPoints);

  float a = 2. * PI * vId / (cPoints - 1.);

  float snd = pow(texture2D(iChannel0, vec2(0.05, cNorm * .125)).x, 4.);
  snd = iBeat*0.1 + snd*0.05;

  float rad = 0.05 + 0.1 * (1. - cNorm) + sin(a * 10.) * (0.05 + 0.3 * snd);
  float x = sin(a) * rad;
  float y = cos(a) * rad;

  //  for(i=0;i<1;i++){
  //if(iY > 0.0){
  //if (iLineMode<=0.0){

  float speed = 0.0;
  if(iSpeed > 0.0){
    speed = iSpeed;
  }
  if(iIntro == 3.0){
    speed = 0.0;
  }

  if(iIntro == 0.0 &&  rand(vec2(vertexId, iResolution.y)) > 0.5){
      y += (cos(time * 1.09 - cId / 159.2) * .3 / aspect) + rand(vec2(vertexId, iResolution.y))*speed  ;
  }
  if(iIntro == 0.0 && rand(vec2(vertexId, iResolution.y)) > 0.9){
      y += (cos(time * 1.09 - cId / 159.2) * .3 / aspect) + rand(vec2(vertexId, iResolution.x))/speed*2  ;
  }

  y += (cos(time * 1.09 - cId / 159.2) * .3 / aspect);// + rand(vec2(vertexId, iResolution.y))*iSpeed  ;
  y += (cos(time * 1.49 - cId / 147.2) * .4 / aspect);
  x += (sin(time * 1.23 + cId / 133.3) * .3); //+ rand2(vec2( iResolution.x, vertexId))*speed*1;
  x += (sin(time * 1.31 + cId / 171.3) * .4);


  gl_Position = vec4(x, y * aspect, 0, 1);
  gl_PointSize =  iBeat*2.0 + 2.2;

  float r = sin(time * 1.42 - cNorm * 8.) * .5 + .5;
  float g = sin(time * 1.27 + a) * .5 + .5;
  float b =  sin(time * 1.12 + cNorm * 6.) * .5 + .5;

  //gl_BackColor = vec4(r*1., g*1., b*1., 1);
  gl_FrontColor = vec4(r*1., g*1., b*1., 1);
}
