#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinates;
layout(location = 3) in vec4 color;

out VS_OUT
{
  vec4 color;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_model;

void main()
{
  vs_out.color = color;
  gl_Position = u_projection * u_model * vec4(position, 1.0);
}