#version 330 core 

layout(location = 0) in vec2 aPos;

out vec2 fragCoord;

void main() 
{
  // aPos is vector location, which since we are doing full screen should just be corners 
  // we map [-1,1] to [0,1]
  // we flip y, so that 0,0 is the top left corner 
  fragCoord = vec2((aPos.x + 1.0) * 0.5, 1.0 - (aPos.y + 1.0) * 0.5);
  gl_Position = vec4(aPos, 0.0, 1.0);
}
