#version 330 core

// Renders the contents of ONE tile into an offscreen FBO.
// Produces exactly the pixels that used to come out of
// renderGradient() / renderRankColor() / raytracer() on the CPU.
//
// The FBO is tile_w x tile_h. gl_FragCoord is bottom-up, so we flip
// Y to get a top-left-origin pixel address within the tile.

uniform vec2 u_tile;     // (local_width,  local_height)  in pixels
uniform vec2 u_global;   // (global_width, global_height) in pixels
uniform vec2 u_offset;   // this tile's top-left origin in the global image
uniform int  u_rank;     // MPI world rank (useful for rank-color debug)

out vec4 fragColor;

void main() {
    // Pixel address with top-left origin (matches the CPU buffers).
    vec2 local = vec2(gl_FragCoord.x, u_tile.y - gl_FragCoord.y);

    // Pixel address in the global composite image.
    vec2 g = u_offset + local;

    // ---- gradient, equivalent to old renderGradient() -------------------
    vec3 c = vec3(g.x / u_global.x,
                  g.y / u_global.y,
                  0.5);

    // ---- subtle checkerboard overlay ------------------------------------
    float checker = mod(floor(g.x / 32.0) + floor(g.y / 32.0), 2.0);
    c *= mix(0.85, 1.0, checker);

    // ---- (later) raytracer, palette, etc. go here -----------------------
    // Example rank-colour debug:
    //   const vec3 kPalette[16] = vec3[16]( ... );
    //   c = kPalette[u_rank % 16];

    fragColor = vec4(c, 1.0);
}
