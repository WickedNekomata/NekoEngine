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
};

struct Light
{
	vec3 direction;
	float intensity;
};

uniform vec3 viewPos;
//uniform Light light;
uniform Material material;

uniform vec3 lineColor; // the silhouette edge color
uniform int levels;

vec3 toonShade(vec3 ambient, vec3 diffuse, float scaleFactor)
{
    vec3 lightDirection = vec3(-0.2, -1.0, -0.3);
    float lightIntensity = 0.8;
    
    vec3 lightDir = normalize(lightDirection);
    vec3 reflectDir = normalize(reflect(fs_in.fNormal, -lightDir));
    vec3 viewDir = normalize(fs_in.fPosition - viewPos);
      
    float cosine = max(0.0, dot(fs_in.fNormal, -lightDir));

    vec3 diffuseColor = diffuse * floor(cosine * levels) * scaleFactor;
    
    vec3 toonColor = lightIntensity * (ambient + diffuseColor); 
    return toonColor;
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
       float scaleFactor = 1.0 / levels;
       
	   vec3 diffuse = vec3(albedo);
	   vec3 ambient = diffuse;
	   vec3 toon = toonShade(ambient, diffuse, scaleFactor);
	   FragColor = vec4(toon, albedo.a);
	}
}