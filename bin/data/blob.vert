
uniform sampler2D iChannel0;

#define PI radians(180.)

vec3 hsv2rgb(vec3 c) {
  c = vec3(c.x, clamp(c.yz, 0.0, 1.0));
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

struct point {
  vec3 position;
  float a;
  float b;
  float rad;
  float snd;
};

point getPoint(float i) {

  float vertexCount= 80000;
  float pointsPerTurn = floor(sqrt(vertexCount) * 2.);
  pointsPerTurn -= mod(pointsPerTurn, 16.);
  pointsPerTurn++;
  float turns = vertexCount / pointsPerTurn;
  
  float a = (PI + PI / turns) * i; //(acos(1.0 - 2.0 * i) / PI);
  float b = 2. * PI * i * turns;
  

  a -= mod(gl_VertexID, 2.) * PI / (turns - 1.);
  a = clamp(a, 0., PI);

  
  float spike = pow(cos(a * 4.), 4.);
  bool middle = a > PI / 8. && a < PI * 7. / 8.;
  if (middle) {
	  spike *= pow(sin(b * 4.), 4.);
  }
  
  float snd = pow(texture2D(iChannel0, vec2(0.005, spike*0.025)).a, 4.);

  float rad = 0.35;    
  rad += spike * 0.35;
  rad += snd * 0.2;  

  
  float x = sin(a);
  float y = cos(a);  
  float z = sin(b) * x;
  x *= cos(b);
      
  return point(vec3(x,  y, z) * rad, a, b, rad, snd);  	
}

void main() {
  float vertexCount= 6000;
  point p1 = getPoint(gl_VertexID / vertexCount);
//  point p2 = getPoint((gl_VertexID - 1.) / vertexCount);
//  point p3 = getPoint((gl_VertexID - 2.) / vertexCount);
//  vec3 normal = normalize(cross(p2.position - p1.position, p3.position - p1.position));
  float time = iGlobalTime;
  
  float mx = sin(time - p1.rad) * 1.4;
//  float mx = PI * -iMouse.y;
  float my = time - p1.rad;
//  float my = PI * -iMouse.x;
  float mz = sin(time * 0.44 - p1.rad);
  mat2 rotateX = mat2(cos(mx), -sin(mx), sin(mx), cos(mx));
  mat2 rotateY = mat2(cos(my), -sin(my), sin(my), cos(my));
  mat2 rotateZ = mat2(cos(mz), -sin(mz), sin(mz), cos(mz));
  p1.position.yz *= rotateX;
  p1.position.xz *= rotateY;
  p1.position.xy *= rotateZ;  
//  normal.xz *= rotateX;
//  normal.yz *= rotateY;
  

  
  float screenZ = -0.;
  float eyeZ = -1.5;
  float perspective = (eyeZ - screenZ) / (p1.position.z - eyeZ);
  p1.position.xy *= perspective;
  
  float aspect = iResolution.x / iResolution.y;
  p1.position.x /= aspect;
  
  gl_Position = vec4(p1.position, 1);
  
  gl_PointSize = 2. - p1.position.z * 5.;


  float h = fract(p1.b / (2. * PI));
  float s = (p1.rad - 0.5 * p1.snd) * 3.;
  if (p1.a < PI / 8.) {
  	s = mix(s, 0., 1. - p1.a / (PI / 8.));
  } else if (p1.a > PI * 7. / 8.) {
  	s = mix(s, 0., 1. - (PI - p1.a) / (PI / 8.));  
  }
  float v = 0.4 - p1.position.z * 2.;
//  float v = normal.z * 0.5 + 0.5; 
  
  vec4 v_color = vec4(hsv2rgb(vec3(h, s, v)), 1);
  gl_FrontColor = v_color;
}