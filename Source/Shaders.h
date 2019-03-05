#ifndef __SHADERS_H__
#define __SHADERS_H__

#pragma region ShaderDefault

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
"	//vec3 a = vec3(albedo);\n" \
"	//vec3 s = vec3(texture(material.specular, fTexCoord));\n" \
"	//vec3 phong = phong(a, a, s, 32.0, true);\n" \
"	FragColor = albedo;\n" \
"}"

#pragma endregion

#pragma region ShaderParticles

#define Particle_vShaderTemplate \
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
"uniform vec4 currColor;\n" \
"uniform float rowUVNorm;\n" \
"uniform float columUVNorm;\n" \
"uniform vec2 currMinCoord;\n" \
"uniform int isAnimated;\n" \
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
"	fColor = currColor;\n" \
"	if(isAnimated == 0)\n" \
"		fTexCoord = texCoord;\n" \
"	else\n" \
"	{\n" \
"		fTexCoord = currMinCoord;\n" \
"		if(texCoord.x == 1)\n" \
"			fTexCoord.x += columUVNorm;\n" \
"		if(texCoord.y == 1)\n" \
"			fTexCoord.y += rowUVNorm;\n" \
"	}\n" \
"\n" \
"	gl_Position = mvp_matrix * vec4(position, 1.0);\n" \
"}"

#define Particle_fShaderTemplate \
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
"uniform float averageColor;\n" \
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
"   vec4 textColor = vec4(phong, albedo.a);\n" \
"	FragColor = mix(textColor, fColor, averageColor);" \
"}"
#pragma endregion

#pragma region ShaderUI
//UI
#define uivShader \
"#version 330 core\n" \
"layout(location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>\n" \
"layout(location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>\n" \
"out vec2 TexCoords;\n" \
"uniform vec2 topRight;\n" \
"uniform vec2 topLeft;\n" \
"uniform vec2 bottomLeft;\n" \
"uniform vec2 bottomRight;\n" \
"void main()\n" \
"{\n" \
"	vec2 position = topRight;\n" \
"	if (vertex.x > 0.0 && vertex.y > 0.0)\n" \
"		position = topRight;\n" \
"	else if (vertex.x > 0.0 && vertex.y < 0.0)\n" \
"		position = bottomRight;\n" \
"	else if (vertex.x < 0.0 && vertex.y > 0.0)\n" \
"		position = topLeft;\n" \
"	else if (vertex.x < 0.0 && vertex.y < 0.0)\n" \
"		position = bottomLeft;\n" \
"	TexCoords = texture_coords;\n" \
"	gl_Position = vec4(position, 0.0, 1.0);\n" \
"}"

#define uifShader \
"#version 330 core\n" \
"in vec2 TexCoords;\n" \
"out vec4 FragColor;\n" \
"uniform int use_color;\n"\
"uniform sampler2D image;\n" \
"uniform vec4 spriteColor;\n" \
"void main()\n" \
"{\n" \
"	if(use_color == 1)\n"\
"		FragColor = spriteColor;\n" \
"	else\n"\
"		FragColor = texture(image, TexCoords);\n" \
"}"

#pragma endregion

#endif