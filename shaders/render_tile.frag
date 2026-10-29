#version 330 core

uniform vec2 u_tile;     // (local_width,  local_height)
uniform vec2 u_global;   // (global_width, global_height)
uniform vec2 u_offset;   // this tile's top-left origin in the global image
uniform int  u_rank;

out vec4 fragColor;


void main() {
    vec2 local = vec2(gl_FragCoord.x, u_tile.y - gl_FragCoord.y);

    vec2 g = u_offset + local;
    fragColor = vec4(1.0 * u_rank,1.0,1.0, 1.0);
}
