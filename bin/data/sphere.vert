
#define PI 3.14159
uniform sampler2D iChannel0;

vec3 project(vec3 p) { return vec3(p.xy/p.z, -p.z); }

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main () {
  float i = rand(vec2(gl_VertexID, 184.98));
  float j = rand(vec2(gl_VertexID, 274.14));
  float f = rand(vec2(gl_VertexID, 387.36));
  
  float snd = pow(texture2D(iChannel0, vec2(.02+.5 * f, 0.)).x, 8.);
  float asp = iResolution.x / iResolution.y;
  
  
  float rad = .2 + .05 * snd;
  float a1 = 2. * PI * i;
  float a2 = 2. * PI * j + iGlobalTime * .05;
  a2 -= iMouse.x;
  
  float y = rad * sin(a1) + .05;
  float xz_rad = rad * cos(a1);
  float x = xz_rad * cos(a2) / asp;
  float z = -.4 + xz_rad * sin(a2);
  z += iMouse.y * .1;

  
  float dist = abs(z) *.5;
  
  vec3 p = project(vec3(x, y, z));
  gl_Position = vec4(p, 1.);
  gl_PointSize = 8. * snd + 1. / dist;
  gl_FrontColor = vec4(
    .2 * acos(f) / PI + .8 * snd,
    .4 * snd * + .3 * asin(f) / PI,
    .6 * snd + .2 * rand(vec2(i, j)),
   	snd);
}