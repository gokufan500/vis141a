#version 330 core

in  vec2      v_uv;
out vec4      fragColor;

uniform sampler2D u_screen;

void main() {
    ivec2 texSize = textureSize(u_screen, 0);
    vec2  dims    = vec2(texSize);

    vec2 g = vec2(v_uv.x * dims.x, (1.0 - v_uv.y) * dims.y);

    vec3 base = texture(u_screen, v_uv).rgb;

    vec3 grad = vec3(g.x / dims.x, g.y / dims.y, 0.2);
    vec3 c = base; 
    //vec3 c    = mix(base, grad, 0.35);

    float fw   = max(fwidth(g.x), fwidth(g.y)) * 1.5 + 0.5;

    vec2 d100  = abs(mod(g + 50.0, 100.0) - 50.0);
    float l100 = 1.0 - smoothstep(0.0, fw, min(d100.x, d100.y));
    //c          = mix(c, vec3(0.15), l100 * 0.5);

    vec2 d500  = abs(mod(g + 250.0, 500.0) - 250.0);
    float l500 = 1.0 - smoothstep(0.0, fw * 2.0, min(d500.x, d500.y));
    //c          = mix(c, vec3(1.0), l500 * 0.7);

    ///if (g.x < 2.0 || g.x > dims.x - 2.0 ||
       // g.y < 2.0 || g.y > dims.y - 2.0)
       //c = vec3(1.0, 0.0, 1.0);

    const float M = 80.0;
    bool tl = g.x < M          && g.y < M;             // RED    top-left
    bool tr = g.x > dims.x - M && g.y < M;             // GREEN  top-right
    bool bl = g.x < M          && g.y > dims.y - M;    // BLUE   bottom-left
    bool br = g.x > dims.x - M && g.y > dims.y - M;    // YELLOW bottom-right

    if (tl) c = vec3(1.0, 0.0, 0.0);
    if (tr) c = vec3(0.0, 1.0, 0.0);
    if (bl) c = vec3(0.0, 0.0, 1.0);
    if (br) c = vec3(1.0, 1.0, 0.0);

    fragColor = vec4(c, 1.0);
}
