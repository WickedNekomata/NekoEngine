#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D ourTexture_0;

uniform float T;


void main()
{
     FragColor = texture(ourTexture_0, ourTexCoord);
}
