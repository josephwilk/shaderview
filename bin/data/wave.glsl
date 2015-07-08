uniform sampler2D iChannel0;
uniform float     iVolume;
uniform float     iExample;

#define PI  3.14159  
#define EPS .001  

#define T .03  // Thickness  
#define W 1.   // Width  
#define A .9  // Amplitude  
#define V 0.0001   // Velocity  


void main(void)
{
vec2 c = gl_FragCoord.xy / iResolution.xy;  
	vec4 s = texture2D(iChannel0, c * .5);  
	c = vec2(0., A*s.y*sin((c.x*W+iGlobalTime*V)* 2.5)) + (c*2.-1.);  
	float g = max(abs(s.y/(pow(c.y, 2.1*sin(s.x*PI))))*T,  
				  abs(.1/(c.y+EPS)));  
	gl_FragColor = vec4(g*g*s.y*(.6+iExample), g*s.w*.44, g*g*.7, 1.);  
} 