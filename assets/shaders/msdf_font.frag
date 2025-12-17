#version 460 core

in VS_OUT
{
  vec2 texCoord;
} fs_in;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D msdfTex;
uniform float pxRange;

float median(float r, float g, float b) 
{
  return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() 
{
  vec2 unitRange = vec2(pxRange) / vec2(textureSize(msdfTex, 0));
  vec2 screenTexSize = vec2(1.0) / fwidth(fs_in.texCoord);
  return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main()
{
  vec3 msd = texture(msdfTex, fs_in.texCoord).rgb;

  float sd = median(msd.r, msd.g, msd.b);

  float screenPxDistance = screenPxRange() * (sd - 0.5);

  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

  if (opacity < 0.01) discard;

  FragColor = vec4(vec3(1.0), opacity);
}