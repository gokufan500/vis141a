#version 330 core

uniform vec2 u_tile;     // (local_width,  local_height)  in pixels
uniform vec2 u_global;   // (global_width, global_height) in pixels
uniform vec2 u_offset;   // this tile's top-left origin in the global image
uniform int  u_rank;     // MPI world rank (useful for rank-color debug)

out vec4 fragColor;

void main() {
    vec2 local = vec2(gl_FragCoord.x, u_tile.y - gl_FragCoord.y);

    vec2 g = u_offset + local;

    vec3 c = vec3(g.x / u_global.x,
                  g.y / u_global.y,
                  0.5);

    float checker = mod(floor(g.x / 32.0) + floor(g.y / 32.0), 2.0);
    c *= mix(0.85, 1.0, checker);

    fragColor = vec4(1.0,0.0,0.0, 1.0);
}
