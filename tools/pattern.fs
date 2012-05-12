uniform float time;
uniform vec2 resolution;
uniform vec4 mouse;

const float tot = 2.0*3.1415926;

void main(void){
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / resolution.xy;
    p.y *= resolution.y/resolution.x;
    p *= 100.0;
    const float n = 9.0;
    const float df = tot/n; 
    float c = 1.;
    float t = time*1.0;

    for (float phi =0.0; phi < tot; phi+=df){
        c+=cos(cos(phi)*p.x+sin(phi)*p.y+t*phi/tot);
    }

    gl_FragColor = vec4(c,c,c,1.0);
}