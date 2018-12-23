#version 330 core

const float eta = 0.15;

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;
in mat3 TBN;
in vec3 startPos;

out vec4 FragColor;

struct Material 
{
sampler2D albedo;
sampler2D specular;
sampler2D normalMap;
float shininess;
};

struct Light 
{
vec3 direction;

vec3 ambient;
vec3 diffuse;
vec3 specular;
};

struct Ripple 
{
float speed;
float amplitude;
float frequency;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;
uniform Ripple ripple;
uniform samplerCube skybox;

vec3 FragNormal;

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)
{
// Ambient
vec3 a = light.ambient * ambient;

// Diffuse
vec3 lightDir = normalize(-light.direction);
float diff = max(dot(FragNormal, lightDir), 0.0);
vec3 d = light.diffuse * (diff * diffuse);

// Specular
vec3 viewDir = normalize(viewPos - fPosition);
float spec = 0.0;
if (blinn)
{
vec3 halfwayDir = normalize(lightDir + viewDir);
spec = pow(max(dot(FragNormal, halfwayDir), 0.0), shininess);
}
else
{
vec3 reflectDir = reflect(-lightDir, FragNormal);
spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
}
vec3 s = light.specular * (spec * specular);

return a + d + s;
}

void main()
{
FragNormal = texture(material.normalMap, fTexCoord).rgb;
FragNormal = normalize(FragNormal * 2.0 - 1.0);   
FragNormal = normalize(TBN * FragNormal); 
    
vec4 albedo = texture(material.albedo, fTexCoord);
vec4 specular = texture(material.specular, fTexCoord);
vec3 a = vec3(albedo);
vec3 s = vec3(specular);

// White on top, blue on bottom
float rippleTop = startPos.y + ripple.amplitude;
float rippleBottom = startPos.y;
vec4 white = vec4(1.0,1.0,1.0,1.0);
vec4 blue = vec4(0.0,0.5,0.8,1.0);

float mixAmount = 0.0;
float range = rippleTop - rippleBottom;
if (range > 0)
mixAmount = (fPosition.y - rippleBottom) / range;
vec4 color = mix(blue, white, mixAmount);

a = vec3(color);

// Reflection and refraction in the water
vec3 lightDir = normalize(-light.direction);
vec3 reflectDir = reflect(-lightDir, FragNormal);
vec3 refractDir = refract(-lightDir, FragNormal, eta);

vec4 reflectionColour = texture(skybox, reflectDir);
reflectionColour.a = 1.0;
vec4 refractionColour = texture(skybox, refractDir);
refractionColour.a = 0.2;

vec3 viewDir = normalize(viewPos - fPosition);
float refractiveFactor = dot(viewDir, FragNormal);

vec4 refColour = mix(reflectionColour, refractionColour, refractiveFactor);

// Final colour
vec3 finalColour = vec3(mix(vec4(a, albedo.a), refColour, 0.5));

vec3 phong = phong(finalColour, finalColour, s, material.shininess, true);
FragColor = vec4(phong, refColour.a);
}