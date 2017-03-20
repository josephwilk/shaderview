vec3 hsv2rgb(vec3 c) {
  c = vec3(c.x, clamp(c.yz, 0.0, 1.0));
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  float t = iGlobalTime * 0.2;
  float NUM_SEGMENTS = 3. + mod(floor(t), 20.);
  float NUM_POINTS = NUM_SEGMENTS * 2.0;
  float STEP = 3.0 + mod(floor(t * 4.), NUM_SEGMENTS);
  float PI = 3.14159 + mod(floor(t * 8.), 30.0);
  float localTime = iGlobalTime + 20.0;
  float point = mod(floor(gl_VertexID / 2.0) + mod(gl_VertexID, 2.0) * STEP, NUM_SEGMENTS);
  float count = floor(gl_VertexID / NUM_POINTS);
  float offset = count * sin(iGlobalTime * 0.01) + 5.0;
  float angle = point * PI * 2.0 / NUM_SEGMENTS + offset;
  float radius = pow(count * 0.00024, 1.0);
  float c = cos(angle + localTime) * radius;
  float s = sin(angle + localTime) * radius;
  float orbitAngle =  pow(count * 0.9, 0.8);
  float innerRadius = pow(count * 0.0025, 1.10);
  float oC = cos(orbitAngle + count * 0.01) * innerRadius;
  float oS = sin(orbitAngle + count * 0.01) * innerRadius;

  vec2 aspect = vec2(1, iResolution.x / iResolution.y); 
  vec2 xy = vec2(
      oC + c,
      oS + s);
  gl_Position = vec4(xy * aspect, 0, 1);

  
  float hue = floor(iGlobalTime) / 0.23;
  float sat = 1.;
  float val = 0.8;
  gl_FrontColor = vec4(hsv2rgb(vec3(hue, sat, val)), 1);
}