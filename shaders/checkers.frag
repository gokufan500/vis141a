#version 330 core

in  vec2      v_uv;
out vec4      fragColor;

uniform sampler2D u_screen;

void main() {
    vec3 base = texture(u_screen, v_uv).rgb;

    // subtle checker overlay in UV space
    vec2  c   = floor(v_uv * vec2(32.0, 32.0));
    float m   = mod(c.x + c.y, 2.0);
    base     *= mix(0.85, 1.0, m);

    fragColor = vec4(base, 1.0);
}
