#version 460 core

in vec2 texCoords;

out vec4 FragColor;

uniform sampler2D msdfTex;
uniform float pxRange;

float median(float r, float g, float b)
{
  return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec3 msd = texture(msdfTex, texCoords).rgb;

  float sd = median(msd.r, msd.g, msd.b);
  // largeur de la transition pour l'anti-aliasing
  float screenPxDistance = pxRange * (sd - 0.5);
  // adapte l'anti-aliasing au zoom
  float opacity = clamp(screenPxDistance / fwidth(screenPxDistance) + 0.5, 0.0, 1.0);

  vec4 textColor = vec4(1.0);
  FragColor = vec4(textColor.rgb, textColor.a * opacity);
  if (FragColor.a < 0.01) discard;
}