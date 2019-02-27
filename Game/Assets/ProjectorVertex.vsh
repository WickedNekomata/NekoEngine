#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

uniform mat4 projector_matrix;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;

out vec4 fProjectorTexCoord;

void main()
{
    vec4 pos4 = vec4(position, 1.0);
    vec4 modelPosition = model_matrix * pos4;

	fPosition = modelPosition.xyz;
	fNormal = normalize(normal_matrix * normal);
	fColor = color;
	fTexCoord = texCoord;	
	fProjectorTexCoord = projector_matrix * modelPosition;

	gl_Position = mvp_matrix * pos4;
}