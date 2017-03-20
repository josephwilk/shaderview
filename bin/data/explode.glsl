const int ITERATIONS = 20;
const float STEP = 0.08;
const float DURATION = 5.0;
uniform sampler2D iChannel0;
float noise(vec3 x) // iq's 3D noise
{
    vec3 f = fract(x);
    vec3 p = x - f;
    f = f*f*(3.0 - 2.0*f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture2D(iChannel0, (uv + 0.5)/256.0, -100.0).rg;
    return mix(rg.y, rg.x, f.z);
}

float fbm(vec3 x)
{
    float r = 0.0;
    float w = 1.0, s = 1.0;
    for (int i=0; i<5; i++)
    {
        w *= 0.5;
        s *= 2.0;
        r += w * noise(s * x);
    }
    return r;
}

vec3 getColor(float r, float d)
{
    vec3 c = mix(vec3(0.9), vec3(0.5, 0.4, 0.1), d);
    
	c *= mix(vec3(1.0, 0.4, 0.0), vec3(0.5), r );
    
	return c * 4.0;
}

vec4 scene(vec3 pos, float t)
{    
    float density = fbm(4.0*pos);
    
    float rad = length(pos);
    density -= 0.1 + rad * (1.0 - 0.2 * t);
   
    density = clamp(density, 0.0, 1.0);
    density *= (1.0 - smoothstep(0.5, 1.0, rad));
    
    return vec4(getColor(rad, density), density);
}

vec3 render(vec3 eye, vec3 dir, float t)
{
    vec4 color = vec4(0);
    
    vec3 pos = eye + dir * STEP * texture2D(iChannel0, gl_FragCoord.xy/iResolution.x).r;
    
    for (int i=0; i<ITERATIONS; i++)
    {
        if (color.a > 0.99) continue;
        if (length(pos) > 1.1) continue;
        
        vec4 d = scene(pos, t);
        d.rgb *= d.a;
        color += d * (1.0 - color.a);
        
        pos += dir * STEP;
    }
    
    return clamp(color.rgb, 0.0, 1.0);
}

vec3 lookAtDir(vec2 uv, vec3 eye, vec3 at, vec3 up, float fov)
{
    vec3 f = normalize(at - eye);
    vec3 r = normalize(cross(up, f));
    vec3 u = normalize(cross(f, r));
    return normalize(f + fov * (uv.x*r + uv.y*u));
}

void main(void )
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
    float angle = iGlobalTime * 0.3;
    vec3 eye = vec3(sin(angle), 0, cos(angle));
    vec3 dir = lookAtDir(uv, eye, vec3(0, 0, 0), vec3(0, 1, 0), 1.0);
    
    float time = mod(iGlobalTime, DURATION);
	
    vec3 color = render(eye, dir, time);
    
    gl_FragColor = vec4(color, 1.0);
}