#ifndef __MODULE_INTERNAL_RES_HANDLER_H__
#define __MODULE_INTERNAL_RES_HANDLER_H__

#include "Module.h"
#include "Globals.h"

#pragma region ShadersTemplate

#define vShaderTemplate \
"#version 330 core\n" \
"\n" \
"layout(location = 0) in vec3 position;\n" \
"layout(location = 1) in vec3 normal;\n" \
"layout(location = 2) in vec4 color;\n" \
"layout(location = 3) in vec2 texCoord;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 mvp_matrix;\n" \
"uniform mat3 normal_matrix;\n" \
"\n" \
"out vec3 fPosition;\n" \
"out vec3 fNormal;\n" \
"out vec4 fColor;\n" \
"out vec2 fTexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"	fPosition = vec3(model_matrix * vec4(position, 1.0));\n" \
"	fNormal = normalize(normal_matrix * normal);\n" \
"	fColor = color;\n" \
"	fTexCoord = texCoord;\n" \
"\n" \
"	gl_Position = mvp_matrix * vec4(position, 1.0);\n" \
"}"

#define fShaderTemplate \
"#version 330 core\n" \
"\n" \
"in vec3 fPosition;\n" \
"in vec3 fNormal;\n" \
"in vec4 fColor;\n" \
"in vec2 fTexCoord;\n" \
"\n" \
"out vec4 FragColor;\n" \
"\n" \
"struct Material\n" \
"{\n" \
"	sampler2D albedo;\n" \
"	sampler2D specular;\n" \
"	float shininess;\n" \
"};\n" \
"\n" \
"struct Light\n" \
"{\n" \
"	vec3 direction;\n" \
"\n" \
"	vec3 ambient;\n" \
"	vec3 diffuse;\n" \
"	vec3 specular;\n" \
"};\n" \
"\n" \
"uniform vec3 viewPos;\n" \
"uniform Light light;\n" \
"uniform Material material;\n" \
"\n" \
"vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)\n" \
"{\n" \
"	// Ambient\n" \
"	vec3 a = light.ambient * ambient;\n" \
"\n" \
"	// Diffuse\n" \
"	vec3 lightDir = normalize(-light.direction);\n" \
"	float diff = max(dot(fNormal, lightDir), 0.0);\n" \
"	vec3 d = light.diffuse * (diff * diffuse);\n" \
"\n" \
"	// Specular\n" \
"	vec3 viewDir = normalize(viewPos - fPosition);\n" \
"	float spec = 0.0;\n" \
"	if (blinn)\n" \
"	{\n" \
"		vec3 halfwayDir = normalize(lightDir + viewDir);\n" \
"		spec = pow(max(dot(fNormal, halfwayDir), 0.0), shininess);\n" \
"	}\n" \
"	else\n" \
"	{\n" \
"		vec3 reflectDir = reflect(-lightDir, fNormal);\n" \
"		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n" \
"	}\n" \
"	vec3 s = light.specular * (spec * specular);\n" \
"\n" \
"	return a + d + s;\n" \
"}\n" \
"\n" \
"void main()\n" \
"{\n" \
"	vec4 albedo = texture(material.albedo, fTexCoord);\n" \
"	if (albedo.a < 0.1)\n" \
"		discard;\n" \
"\n" \
"	vec3 a = vec3(albedo);\n" \
"	vec3 s = vec3(texture(material.specular, fTexCoord));\n" \
"	vec3 phong = phong(a, a, s, 32.0, true);\n" \
"	FragColor = vec4(phong, albedo.a);\n" \
"}"

// TODO: move operation to ignore translation at view to renderer and do it on cpu. we wanna do it once

#define cubemapvShader \
"#version 330 core\n" \
"layout (location = 0) in vec3 position;\n" \
"uniform mat4 view_matrix;\n" \
"uniform mat4 proj_matrix;\n" \
"out vec3 ourTexCoord;\n" \
"void main()\n" \
"{\n" \
"	 mat3 mat3View = mat3(view_matrix); \n" \
"	 mat4 noTranslationView = mat4(mat3View); \n" \
"    ourTexCoord = position * vec3(1.0,-1.0,1.0);\n" \
"    vec4 pos = proj_matrix * noTranslationView * vec4(position, 1.0f);\n" \
"	 gl_Position = pos.xyww;\n" \
"}"

#define cubemapfShader \
"#version 330 core\n" \
"out vec4 FragColor\n;" \
"in vec3 ourTexCoord\n;" \
"uniform samplerCube skybox\n;" \
"void main()\n" \
"{\n" \
"	FragColor = texture(skybox, ourTexCoord);\n" \
"}"

#pragma endregion

class ModuleInternalResHandler : public Module
{
public:
	bool Start();

	void CreatePlane();
	void CreateCube();

	void CreateDefaultShaderProgram();
	void CreateCubemapShaderProgram();

public:
	//RESOURCES

	// Mesh resources
	uint defaultPlane;
	uint defaultCube;

	// Texture resources
	uint checkers;
	uint defaultTexture;
	// todo cubemap textures

	// Shader resources
	uint defaultShaderProgram;
	uint cubemapShaderProgram;
};

#endif