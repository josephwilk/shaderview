uniform mat4 modelViewProjectionMatrix;

void main(){
  vec4 pos = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
  gl_Position = pos;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_FrontColor = gl_Color;
}
