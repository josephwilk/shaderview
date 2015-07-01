// vertex shader
#version 120

uniform mat4 modelViewProjectionMatrix;

void main(){
   gl_Position = position * modelViewProjectionMatrix;
}
