#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

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

uniform vec3 viewPos;
uniform Light light;
uniform Material material;
uniform vec3 l;

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
	vec4 albedo = texture(material.albedo, fTexCoord);
	if (albedo.a < 0.1)
		discard;

	vec3 a = vec3(albedo);
	vec3 s = vec3(texture(material.specular, fTexCoord));
	vec3 phong = phong(a, a, s, 32.0, true);
	FragColor = vec4(phong, albedo.a);
}