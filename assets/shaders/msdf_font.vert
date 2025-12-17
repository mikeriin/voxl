#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out VS_OUT
{
  vec2 texCoord;
} vs_out;

uniform mat4 projection;

void main()
{
  vs_out.texCoord = uv;
  gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}