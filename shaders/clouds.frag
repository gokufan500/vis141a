#version 330 core 

in vec2 fragCoord; 
out vec4 FragColor; 

uniform vec2 uGlobalSize;
uniform float uTime; 

// fractal brownian motion from, which we use to test clouddensity, before creating our own 
// perlin woorley noise on cpu to pass as texture 
// https://wiki.methodox.io/Frameworks/ShaderLab/Examples/RaymarchingCloud/#live-demo


#define CLOUD_BOTTOM -0.6
#define CLOUD_TOP     3.0 
float hash(vec3 p)
{
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash(i + vec3(0,0,0));
    float n100 = hash(i + vec3(1,0,0));
    float n010 = hash(i + vec3(0,1,0));
    float n110 = hash(i + vec3(1,1,0));
    float n001 = hash(i + vec3(0,0,1));
    float n101 = hash(i + vec3(1,0,1));
    float n011 = hash(i + vec3(0,1,1));
    float n111 = hash(i + vec3(1,1,1));

    return mix(
        mix(mix(n000, n100, f.x), mix(n010, n110, f.x), f.y),
        mix(mix(n001, n101, f.x), mix(n011, n111, f.x), f.y),
        f.z
    );
}

float fbm(vec3 p)
{
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; i++)
    {
        v += a * noise(p);
        p *= 2.02;
        a *= 0.5;
    }
    return v;
}

float cloudDensity(vec3 p) {

  p.xz += vec2(uTime * 1.5, uTime * 0.2); 

  // large scale shape 
  float shape = fbm(p * 5.0); 

  float coverage = smoothstep(0.40, 0.72, shape); 
  if (coverage <= 0.001) return 0.0; 

  float topY = mix(0.5, CLOUD_TOP, coverage);
  float heightMask = smoothstep(CLOUD_BOTTOM, 0.3, p.y) * (1.0 - smoothstep(topY  - 1.0 , topY, p.y));
  if (heightMask <= 0.001) return 0.0; 

  // smaller detail to give texture 
  float detail = fbm(p * 1.45 + 11.0);
  float d = coverage - 0.42 * detail * (1.0 - coverage);

  return clamp(d * heightMask * 2.0, 0.0, 1.0);
}

mat3 camera(vec3 ro, vec3 ta) {
  // ray origin, target 
  vec3 f = normalize(ta - ro);
  vec3 r = normalize(cross(vec3(0,1,0), f));
  vec3 u = cross(f, r);
  return mat3(r,u,f);
}
vec3 rayDirection(vec2 uv, mat3 cam) {
    vec2 ndc = uv * 2.0 - 1.0;
    return normalize(cam * vec3(ndc, 1.5));
}

// henyey greenstein phase function 
float hgPhase(float cosTheta, float g) {
  float g2 = g*g;
  return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// at each point, we march towards the sun to get transmittance from sun, not just ambient 
float lightMarch(vec3 p, vec3 sunDir) {
  const int   LIGHT_STEPS = 6;
  const float LIGHT_STEP  = 20.0;
  float shadowT = 0.15 ; 
  float depth = 0.0; 
  for (int j = 0; j < LIGHT_STEPS; ++j) {
    depth += cloudDensity(p + sunDir * shadowT) * shadowT;
    shadowT *= 1.9;
  }
  return depth; 
}

void main() {
  vec2 uv = (fragCoord - 0.5);
  uv.x * uGlobalSize.x / uGlobalSize.y; 

  vec3 ro = vec3(0.0, 6, -14.0);
  vec3 ta = vec3(0.0, 2.5, 3.0);

  mat3 cam = camera(ro,ta);
  vec3 rd = normalize(cam * vec3(uv, 1.5));
  
  
  // simple sky gradient 
  float skyGradient = clamp(rd.y * 0.5 + 0.5, 0.0, 1.0);

  vec3 skyHorizon = vec3(0.70, 0.82, 0.95); // pale blue at horizon 
  vec3 skyZenith  = vec3(0.15, 0.38, 0.75); // deeper blue in sky 
  vec3 skyColor   = mix(skyHorizon, skyZenith, skyGradient);


  // this will accumulate in scattered light along ray 
  vec3 col = vec3(0.0);

  // transmittance, the amount of light still reaching the camera 
  // as we pass through cloud. this value will decrease as more 
  // light hits cloud particles and scatters 
  float transmittance = 1.0; 
  
  vec3 sunDir   = normalize(vec3(0.82,0.26,0.28));
  vec3 sunColor = vec3(1.0,0.95,0.85);
  float cosTheta = dot(rd, sunDir); // angle between ray and sun 
 
  float sunDisk = smoothstep(0.999, 0.9999, dot(rd, sunDir));
  

  // dual lobe, forward lobe for silver lining, backward lobe for soft body
  // approximates multiple scattering for cheap
  float phase = 0.7 * hgPhase(cosTheta, 0.76) + 0.3 * hgPhase(cosTheta, -0.28);

  // clip march to cloud slab 
  float tEnter = 0.0; 
  float tExit  = -1.0; 

  if (abs(rd.y) > 1e-4){
    float tA = (CLOUD_TOP - ro.y) / rd.y; 
    float tB = (CLOUD_BOTTOM - ro.y) / rd.y;
    tEnter = max(0.0, min(tA, tB));
    tExit  = min(90.0, max(tA, tB));
  } else if (ro.y > CLOUD_BOTTOM && ro.y < CLOUD_TOP){
    tEnter = 0.0;
    tExit = 90.0;
  }
  // raymarching step 
  const int   STEPS    = 112;
  const float MAX_DIST = 90.0;
  float stepSize = MAX_DIST / float(STEPS);
  
  if (tExit > tEnter){ 

    float stepLen = max((tExit - tEnter) / float(STEPS), 0.05);

    // distance traveled at point in loop 
    float t = tEnter + hash(vec3(fragCoord, uTime)) * stepLen;
   
    for (int i = 0; i < STEPS; ++i){
      
      if (t > tExit || transmittance < 0.01) break;
      // 3D world position, direction * distance, from the origin 
      vec3 p = ro + rd * t; 
      
      // sigma is extinction coeffecient, how much light is lost 
      float sigma = cloudDensity(p);
      
      if (sigma > 0.005) {
        // naive implementation 
        // col           += transmittance * sigma * vec3(1.0);
        // transmittance *= (1.0 - d);
        
        // Scattering Source Function, at point p 
        // S(p,w) = sigma(p) * p(theta) * Lsun * Tshadow(p)
        // p(theta) phase function (Henyey-Greenstein)
        float shadowT = exp(-lightMarch(p, sunDir) * 2.9);
        float powder = 1.0 - exp(-sigma * 9.0);
        vec3 sunLight = vec3(1.0, 0.88, 0.70) * shadowT * (phase * 2.2 + 0.50) * powder;
        vec3 ambient  = mix(vec3(0.20, 0.26, 0.40), vec3(0.74, 0.82, 0.98), smoothstep(CLOUD_BOTTOM, CLOUD_TOP, p.y)) * 0.30;
        
        vec3 S = (ambient + phase * sunLight * shadowT) * sigma; 

        // volumetric integration from sebastian hillaire 
        float dTrans = exp(-sigma * stepLen); // transmittance through this segment 
        vec3 Sint    = (S - S * dTrans) / sigma; 

        col           += transmittance * Sint; 
        transmittance *= dTrans; 
      }
      t += stepLen;
    }
  }
  // composite cloud over the sky 
  vec3 finalColor = col + skyColor * transmittance;
  finalColor += sunColor * sunDisk;
  FragColor = vec4(finalColor, 1.0);
}
