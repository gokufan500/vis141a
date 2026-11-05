#version 330 core
in vec2 fragCoord;
out vec4 FragColor;

uniform vec2 uTileOffset;  // top-left of this tile in global pixels
uniform vec2 uTileSize;    // size of this tile in pixels
uniform vec2 uGlobalSize;  // total wall size in pixels
uniform vec2 uTileIndex;   // (col, row) of this tile
uniform vec2 uGridSize;    // (cols, rows)
uniform float u_time;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // Local pixels, top-left origin (flip fragCoord.y)
    vec2 localPx  = vec2(fragCoord.x, 1.0 - fragCoord.y) * uTileSize;
    vec2 globalPx = uTileOffset + localPx;
    vec2 uv       = globalPx / uGlobalSize;   // 0..1 across the entire wall

    // Continuous rainbow across the whole wall
    float hue = fract((uv.x + uv.y) * 0.5 + 0.25 * sin(u_time * 0.5));
    vec3 col  = hsv2rgb(vec3(hue, 0.9, 1.0));

    // Large checker so alignment is obvious
    vec2 cc      = floor(uv * 8.0);
    float checker = mod(cc.x + cc.y, 2.0);
    col *= mix(1.0, 0.35, checker);

    // Tile boundary lines (2px on every edge of this tile)
    vec2 inTile  = mod(globalPx, uTileSize);
    float lineW  = 2.0;
    bool onEdge  = inTile.x < lineW || inTile.y < lineW ||
                   inTile.x > uTileSize.x - lineW || inTile.y > uTileSize.y - lineW;
    if (onEdge) col = vec3(1.0);

    // Per-tile color marker in the top-left corner — proves each display
    // is rendering its correct quadrant.
    vec2 markerSize = vec2(60.0, 60.0);
    if (inTile.x < markerSize.x && inTile.y < markerSize.y) {
        vec3 tileCol = hsv2rgb(vec3(fract(uTileIndex.x * 0.25 + uTileIndex.y * 0.5), 1.0, 1.0));
        vec2 cell = floor(inTile / 15.0);
        float m   = mod(cell.x + cell.y, 2.0);
        col = mix(tileCol, vec3(0.0), m * 0.5);
    }

    FragColor = vec4(col, 1.0);
}
