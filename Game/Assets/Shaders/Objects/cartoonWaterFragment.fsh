#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;
in vec3 startPos;

out vec4 FragColor;

struct Material 
{
sampler2D albedo;
sampler2D specular;
float shininess;
};

struct Light 
{
vec3 direction;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

struct Wave 
{
float speed;
float amplitude;
float frequency;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;
uniform Wave wave;

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)
{
// Ambient
vec3 a = light.ambient * ambient;

// Diffuse
vec3 lightDir = normalize(-light.direction);
float diff = max(dot(fNormal, lightDir), 0.0);
vec3 d = light.diffuse * (diff * diffuse);

// Specular
vec3 viewDir = normalize(viewPos - fPosition);
float spec = 0.0;
if (blinn)
{
vec3 halfwayDir = normalize(lightDir + viewDir);
spec = pow(max(dot(fNormal, halfwayDir), 0.0), shininess);
}
else
{
vec3 reflectDir = reflect(-lightDir, fNormal);
spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
}
vec3 s = light.specular * (spec * specular);

return a + d + s;
}

void main()
{
vec3 a = vec3(texture(material.albedo, fTexCoord));
vec3 s = vec3(texture(material.specular, fTexCoord));

// White on top, blue on bottom
float waveTop = startPos.y + wave.amplitude;
float waveBottom = startPos.y;
vec4 white = vec4(1.0,1.0,1.0,1.0);
vec4 blue = vec4(0.0,0.0,1.0,1.0);

float mixAmount = 0.0;
float range = waveTop - waveBottom;
if (range > 0)
mixAmount = (fPosition.y - waveBottom) / range;
vec4 color = mix(blue, white, mixAmount);

a = vec3(mix(color, vec4(a, 1.0), 0.5));

vec3 phong = phong(a, a, s, material.shininess, true);
FragColor = vec4(phong, 0.8);
}