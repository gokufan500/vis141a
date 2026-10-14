#version 330 core 

in vec2 vUV; // from vertex shader 

out vec4 FragColor; // write out

uniform sampler2D uTexture; 

void main() {
  //FragColor = vec4(vUV.x, vUV.y, 0.0, 1.0);
  FragColor = texture(uTexture, vUV);
}
