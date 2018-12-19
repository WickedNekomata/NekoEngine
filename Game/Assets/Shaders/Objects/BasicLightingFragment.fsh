#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

struct Material {
sampler2D ourTexture_0; // diffuse (and ambient)
sampler2D ourTexture_1; // specular
float shininess;
};

struct Light {
vec3 direction;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
// Ambient
vec3 ambient = light.ambient * vec3(texture(material.ourTexture_0, fTexCoord));

// Diffuse
vec3 lightDir = normalize(-light.direction);
float diff = max(dot(fNormal, lightDir), 0.0);
vec3 diffuse = light.diffuse * diff * vec3(texture(material.ourTexture_0, fTexCoord));

// Specular
vec3 viewDir = normalize(viewPos - fPosition);
vec3 reflectDir = reflect(-lightDir, fNormal);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * spec * vec3(texture(material.ourTexture_1, fTexCoord));

vec3 result = ambient + diffuse + specular;
FragColor = vec4(result, 1.0);
}