#version 330 core

precision highp float;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform int patata;
uniform vec2 sexy;

out vec4 ourColor;
out vec2 ourTexCoord;
out vec3 snowDirection;
out float snowQuantity;
out vec3 fNormals;

float height = 0.04;

void main()
{
  ourTexCoord = texCoord;
  ourColor = color;
  fNormals = normals;
  snowDirection = vec3(0.0,1.0,0.0);
  int cola;
  cola = patata;
  snowQuantity = 0.4;
  vec2 yeah;
  yeah = sexy;
  vec3 pos = position;
  if (dot(normals, snowDirection) > snowQuantity)
       pos += normals * height;
  gl_Position = proj_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
}