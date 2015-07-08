// vertex shader
#version 120
uniform mat4 modelViewProjectionMatrix;

void main(){
   gl_Position = gl_Vertex;
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_FrontColor = gl_Color;
}
