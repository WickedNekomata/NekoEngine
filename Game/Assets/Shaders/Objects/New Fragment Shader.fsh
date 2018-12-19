#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D ourTexture_0;

struct Light {
vec3 direction;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

uniform vec3 viewPos;
uniform Light light;

void main()
{
vec3 objectColor = vec3(1.0,0.5,0.31);

float ambientStrength = 0.1;
vec3 ambient = ambientStrength * light.ambient;

vec3 norm = normalize(fNormal);
vec3 lightDir = -normalize(light.direction);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * light.diffuse;

float specularStrength = 1.0;
vec3 viewDir = normalize(viewPos - fPosition);
vec3 reflectDir = reflect(-lightDir, norm); 
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * light.specular;  

vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
}
