uniform float iMotion;

#define PI 3.14159
#define NUM_SEGMENTS 21.0
#define NUM_POINTS (NUM_SEGMENTS * 3.0)
#define STEP 8.0
//#define FIT_VERTICAL

vec3 hsv2rgb(vec3 c) {
  c = vec3(c.x, clamp(c.yz, 1.0, 1.0));
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
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