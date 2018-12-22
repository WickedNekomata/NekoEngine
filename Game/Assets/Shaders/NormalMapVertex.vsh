#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;
out mat3 TBN;

void main()
{
    vec3 T = normalize(vec3(model_matrix * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model_matrix * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model_matrix * vec4(normal,    0.0)));
    TBN = mat3(T, B, N);
       
	fPosition = vec3(model_matrix * vec4(position, 1.0));
	fColor = color;
	fTexCoord = texCoord;

	gl_Position = mvp_matrix * vec4(position, 1.0);
}