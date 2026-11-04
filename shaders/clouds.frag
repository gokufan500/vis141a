#version 330 core 

in vec2 fragCoord; 
out vec4 FragColor; 

uniform vec2 uGlobalSize;
uniform float uTime; 

vec3 rayDirection(vec2 uv){
  // normalized device coordinates 
  vec2 ndc = uv * 2.0 - 1.0;
  // z controls fov, larger z is narrower fov 
  vec3 rd = normalize(vec3(ndc.x, ndc.y, 1.5));
  return rd; 
}

void main() {
  vec2 uv = fragCoord; 
  // ray origin 
  vec3 ro = vec3(0.0,0.0,0.0);
  vec3 rd = rayDirection(uv);

  // raymarching step 
  const int   STEPS    = 64;
  const float MAX_DIST = 1000.0;
  float stepSize = MAX_DIST / float(STEPS);
  // this will accumulate in scattered light along ray 
  vec3 col = vec3(0.0);

  // transmittance, the amount of light still reaching the camera 
  // as we pass through cloud. this value will decrease as more 
  // light hits cloud particles and scatters 
  float transmittance = 1.0; 

  for (int i = 0; i < STEPS; ++i){
    // distance traveled at point in loop 
    float t = float(i) * stepSize;
    
    // 3D world position, direction * distance, from the origin 
    vec3 p = ro + rd * t; 
    
    // density at p 
    float r = length(p - vec3(0.0,0.0,100.0));
    float d = r < 50.0 ? smoothstep(50.0, 0.0 , r) : 0.0;
    float sigma = d * 1;
    // accumulate into col, and reduce transmittance, more light is scattered 
    col           += transmittance * sigma * vec3(1.0);
    transmittance *= (1.0 - d);
    
    if (transmittance < 0.01) break; 
  } 
  FragColor = vec4(col, 1.0);
}
