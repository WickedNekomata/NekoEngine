#version 330 core

precision highp float;

in vec4 ourColor;
in vec2 ourTexCoord;
in vec3 snowDirection;
in float snowQuantity;
in vec3 fNormals;
out vec4 FragColor;

uniform sampler2D ourTexture_0;

vec4 snowColor = vec4(1.0,1.0,1.0,1.0);

void main()
{
  FragColor = vec4(1.0,0.0,0.0,1.0);
  
  if (dot(fNormals, snowDirection) > snowQuantity)
     FragColor = vec4(snowColor);
}