#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;



out vec4 FragColor;

uniform sampler2D ourTexture_0;

void main()
{
vec3 objectColor = vec3(1.0,0.5,0.31);
vec3 lightColor = vec3(1.0,1.0,1.0);
vec3 lightPos = vec3(1.0,1.0,1.0);

float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;

vec3 norm = normalize(fNormal);
vec3 lightDir = normalize(lightPos - fPosition);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

vec3 result = (ambient + diffuse) * objectColor;
FragColor = vec4(result, 1.0);
}
