#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

struct Material {
vec3 ambient;
vec3 diffuse;
vec3 specular;
float shininess;
};

struct Light {
vec3 position;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

uniform float Time;
uniform vec3 viewPos;

void main()
{
Material material;
Light light;

light.position = vec3(1.0,1.0,1.0);
light.ambient = vec3(0.2,0.2,0.2);
light.diffuse = vec3(0.5,0.5,0.5);
light.specular = vec3(1.0,1.0,1.0);

material.ambient = vec3(1.0,0.5,0.31);
material.diffuse = vec3(1.0,0.5,0.31);
material.specular = vec3(0.5,0.5,0.5);
material.shininess = 0.1;

// Ambient
vec3 ambient = light.ambient * material.ambient;

// Diffuse
vec3 lightDir = normalize(light.position - fPosition);
float diff = max(dot(fNormal, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * material.diffuse);

// Specular
vec3 viewDir = normalize(viewPos - fPosition);
vec3 reflectDir = reflect(-lightDir, fNormal);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * material.specular);

vec3 result = ambient + diffuse + specular;
FragColor = vec4(result, 1.0);
}