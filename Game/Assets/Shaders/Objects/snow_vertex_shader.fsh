#version 330 core

precision highp float;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 normals;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

out vec4 ourColor;
out vec2 ourTexCoord;
out vec3 snowDirection;
out float snowQuantity;

float height = 0.06;

void main()
{
  ourTexCoord = texCoord;
  ourColor = color;
  snowDirection = vec3(0.0,1.0,0.0);
  snowQuantity = 0.4;
    
  vec3 pos = position;
  if (dot(normals, snowDirection) > snowQuantity)
      pos += normals * height;
  
  gl_Position = proj_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
}