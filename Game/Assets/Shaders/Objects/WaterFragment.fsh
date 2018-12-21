#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

in vec3 startPos;

out vec4 FragColor;

struct Material {
sampler2D ambient;
sampler2D diffuse;
sampler2D specular;
float shininess;
};

struct Light {
vec3 direction;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

struct Wave {
float speed;
float height;
float amount;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;
uniform Wave wave;

vec3 Phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
// Ambient
vec3 a = light.ambient * ambient;

// Diffuse
vec3 lightDir = normalize(-light.direction);
float diff = max(dot(fNormal, lightDir), 0.0);
vec3 d = light.diffuse * (diff * diffuse);

// Specular
vec3 viewDir = normalize(viewPos - fPosition);
vec3 reflectDir = reflect(-lightDir, fNormal);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 s = light.specular * (spec * specular);

return a + d + s;
}

void main()
{
vec3 a = vec3(texture(material.ambient, fTexCoord));
vec3 d = vec3(texture(material.diffuse, fTexCoord));
vec3 s = vec3(texture(material.specular, fTexCoord));

float waveTop = startPos.y + wave.height;
float waveBottom = startPos.y;
vec4 white = vec4(1.0,1.0,1.0,1.0);
vec4 blue = vec4(0.0,0.0,1.0,1.0);

float mixAmount = 0.0;
float range = waveTop - waveBottom;
if (range > 0)
mixAmount = (fPosition.y - waveBottom) / range;
vec4 color = mix(blue, white, mixAmount);

a = vec3(mix(color, vec4(a, 1.0), 0.5));
d = vec3(mix(color, vec4(d, 1.0), 0.5));

vec3 Phong = Phong(a, d, s, material.shininess);
vec4 result = vec4(Phong, 0.9);
FragColor = result;
}