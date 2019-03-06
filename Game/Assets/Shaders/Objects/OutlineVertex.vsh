#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

out VS_OUT
{
  vec3 gPosition;
  vec3 gNormal;
  vec4 gColor;
  vec2 gTexCoord;
} vs_out;

void main()
{
	vs_out.gPosition = vec3(model_matrix * vec4(position, 1.0));
	vs_out.gNormal = normalize(normal_matrix * normal);
	vs_out.gColor = color;
	vs_out.gTexCoord = texCoord;

	gl_Position = mvp_matrix * vec4(position, 1.0);
}