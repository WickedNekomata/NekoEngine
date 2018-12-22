#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;
in mat3 TBN;

out vec4 FragColor;


struct Material {
	sampler2D albedo;
	sampler2D specular;
	sampler2D normalMap;
	float shininess;
};

struct Light {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

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

	vec3 a = vec3(texture(material.albedo, fTexCoord));
	vec3 s = vec3(texture(material.specular, fTexCoord));
	vec3 phong = phong(a, a, s, 32.0, true);
	FragColor = vec4(phong, 1.0);
}