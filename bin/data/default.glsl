
uniform float iR;
uniform float iG;
uniform float iB;

void main(void){
  vec2 uv = 0.275 * gl_FragCoord.xy / iResolution.y;
  gl_FragColor = vec4(sin(iGlobalTime*uv.x)*0.5+0.5,
                      iG,iB, 1.0);
}