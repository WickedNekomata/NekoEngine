#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;

void main()
{
    mat4 modelViewMatrix = view_matrix * model_matrix;
    
    fPosition = vec3(modelViewMatrix * vec4(position, 1.0));    
    mat3 normal_matrix = mat3(transpose(inverse(model_matrix)));
    fNormal = normalize(normal_matrix * normal);
    fColor = color;
    fTexCoord = texCoord;
    
    gl_Position = proj_matrix * vec4(fPosition, 1.0);
}