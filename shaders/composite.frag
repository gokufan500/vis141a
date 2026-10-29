#version 330 core

// Display process only. Samples the composite texture that the CPU
// assembled from local + peer tile pixels and draws it to the window.
//
// If you want per-frame post-processing (vignette, crosshair, tone-map),
// this is where it goes — NOT in render_tile.frag.

in  vec2      v_uv;
out vec4      fragColor;

uniform sampler2D u_screen;

void main() {
    fragColor = vec4(texture(u_screen, v_uv).rgb, 1.0);
}
