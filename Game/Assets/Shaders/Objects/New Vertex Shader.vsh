#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

out vec4 ourColor;
out vec2 ourTexCoord;

void main()
{
    ourTexCoord = texCoord;
    ourColor = color;
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
}
