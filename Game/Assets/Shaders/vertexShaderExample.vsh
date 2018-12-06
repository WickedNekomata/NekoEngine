#version 330 core
layout (location = 0) in vec2 Position;
layout (location = 1) in vec4 Color;
layout (location = 2) in vec2 texCoord;
uniform mat4 ProjMtx;
out vec4 ourColor;
out vec2 TexCoord;
void main()
{
	TexCoord = texCoord;
	ourColor = color;
	gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
} 