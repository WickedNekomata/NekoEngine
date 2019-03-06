#version 330 core

in GS_OUT
{
  vec3 fPosition;
  vec3 fNormal;
  vec4 fColor;
  vec2 fTexCoord;
} fs_in;

flat in int fIsEdge; // whether or not we're drawing an edge

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

uniform vec3 lineColor; // the silhouette edge color

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)
{
	// Ambient
	vec3 a = light.ambient * ambient;

	// Diffuse
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(fs_in.fNormal, lightDir), 0.0);
	vec3 d = light.diffuse * (diff * diffuse);

	// Specular
	vec3 viewDir = normalize(viewPos - fs_in.fPosition);
	float spec = 0.0;
	if (blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(fs_in.fNormal, halfwayDir), 0.0), shininess);
	}
	else
	{
		vec3 reflectDir = reflect(-lightDir, fs_in.fNormal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}
	vec3 s = light.specular * (spec * specular);

	return a + d + s;
}

void main()
{
	vec4 albedo = texture(material.albedo, fs_in.fTexCoord);
	if (albedo.a < 0.1)
		discard;
		
	// If we're drawing an edge, use constant color
    if (fIsEdge == 1)
    {
	   FragColor = vec4(lineColor, 1.0);
    }
    // Otherwise, shade the poly
    else
    {
	   vec3 a = vec3(albedo);
	   vec3 s = vec3(texture(material.specular, fs_in.fTexCoord));
	   vec3 phong = phong(a, a, s, 32.0, true);
	   FragColor = vec4(a, albedo.a);
	}
}