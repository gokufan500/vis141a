#version 330 core

in vec2 fragCoord; // Normalized 0-1 coordinates
out vec4 FragColor;
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
void main() {
    vec2 uv = fragCoord;
    
    // Rainbow based on position
    float hue = fract((uv.x + uv.y) * 0.5);
    vec3 rainbow = hsv2rgb(vec3(hue, 1.0, 1.0));
    
    // Checker mask (0 or 1)
    vec2 checkerCoord = floor(uv * 10.0);
    float checker = mod(checkerCoord.x + checkerCoord.y, 2.0);
    
    // Remap checker from (0,1) to (0.35, 1.0) for dark squares
    float brightness = mix(1.0, 0.35, checker);
    
    // Or inverted version:
    // vec3 inverted = 1.0 - rainbow;
    // vec3 finalColor = mix(rainbow, inverted, checker);
    
    FragColor = vec4(rainbow * brightness, 1.0);
}
