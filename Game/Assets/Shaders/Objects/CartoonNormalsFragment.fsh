#version 330 core

in vec3 fPosition;
vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;
in mat3 TBN;
out vec4 FragColor;

struct Material
{
	sampler2D albedo;
	sampler2D normalMap;
};

struct Light
{
	vec3 direction;
	float intensity;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

uniform int levels;

vec3 toonShade(vec3 ambient, vec3 diffuse, float scaleFactor)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 reflectDir = normalize(reflect(fNormal, -lightDir));
    vec3 viewDir = normalize(fPosition - viewPos);
      
    float cosine = max(0.0, dot(fNormal, -lightDir));
    //float cosine = max(0.0, dot(reflectDir, fNormal)); 
    vec3 diffuseColor = diffuse * floor(cosine * levels) * scaleFactor;   
    vec3 toonColor = light.intensity * (ambient + diffuseColor);
    //toonColor = mix(toonColor, vec3(1.0, 1.0, 1.0), spec);
    
    return toonColor;
}

void main()
{
    fNormal = texture(material.normalMap, fTexCoord).rgb;
    fNormal = normalize(fNormal * 2.0 - 1.0);   
    fNormal = normalize(TBN * fNormal); 
    //FragColor = vec4(fNormal, 1.0);
    
    float scaleFactor = 1.0 / levels;

	vec4 albedo = texture(material.albedo, fTexCoord);
	if (albedo.a < 0.1)
		discard;

	vec3 diffuse = vec3(albedo);
	vec3 ambient = diffuse;
	vec3 toon = toonShade(ambient, diffuse, scaleFactor);
	FragColor = vec4(toon, albedo.a);
}