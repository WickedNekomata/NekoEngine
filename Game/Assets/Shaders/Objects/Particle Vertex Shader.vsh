#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

uniform float rowUVNorm;
uniform float columUVNorm;
uniform vec2 currMinCoord;
uniform int isAnimated;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;

void main()
{
	fPosition = vec3(model_matrix * vec4(position, 1.0));
	fNormal = normalize(normal_matrix * normal);
	fColor = color;
	
	if(isAnimated == 0)
		fTexCoord = texCoord;
	else
	{
		fTexCoord = currMinCoord;
		if(texCoord.x == 1)
			fTexCoord.x += columUVNorm;
		if(texCoord.y == 1)
			fTexCoord.y += rowUVNorm;
	}

	gl_Position = mvp_matrix * vec4(position, 1.0);
}