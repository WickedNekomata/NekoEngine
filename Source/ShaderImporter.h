#ifndef __SHADER_IMPORTER_H__
#define __SHADER_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

#include "ResourceShaderObject.h"

#include "glew\include\GL\glew.h"

#pragma region ShadersTemplate

#define vShaderTemplate \
"#version 330 core\n" \
"\n" \
"layout (location = 0) in vec3 position;\n" \
"layout (location = 1) in vec4 normals;\n" \
"layout (location = 2) in vec4 color;\n" \
"layout (location = 3) in vec2 texCoord;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 view_matrix;\n" \
"uniform mat4 proj_matrix;\n" \
"\n" \
"out vec4 ourColor;\n" \
"out vec2 ourTexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"    ourTexCoord = texCoord;\n" \
"    ourColor = color;\n" \
"    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);\n" \
"}\n"

#define fShaderTemplate \
"#version 330 core\n" \
"\n" \
"in vec4 ourColor;\n" \
"in vec2 ourTexCoord;\n" \
"out vec4 FragColor;\n" \
"\n" \
"uniform sampler2D ourTexture_0;\n" \
"\n" \
"void main()\n" \
"{\n" \
"     FragColor = texture(ourTexture_0, ourTexCoord);\n" \
"}\n" \

#pragma endregion

class Resource;
class ResourceShaderObject;
class ResourceShaderProgram;

class ShaderImporter : public Importer
{
public:

	ShaderImporter();
	~ShaderImporter();
	
	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const { return true; }
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const { return true; }

	// Shader Object (save)
	bool SaveShaderObject(ResourceShaderObject* shaderObject, std::string& outputFile) const;
	bool SaveShaderObject(const void* buffer, uint size, std::string& outputFile) const;

	// Shader Program (save)
	bool SaveShaderProgram(ResourceShaderProgram* shaderProgram, std::string& outputFile) const;
	bool SaveShaderProgram(const void* buffer, uint size, std::string& outputFile) const;

	bool GenerateMeta(Resource* resource, std::string& outputMetaFile) const;
	bool SetShaderUUIDToMeta(const char* metaFile, uint& UUID) const;
	bool GetShaderUUIDFromMeta(const char* metaFile, uint& UUID) const;

	// Shader Object (load)
	bool LoadShaderObject(const char* objectFile, ResourceShaderObject* shaderObject) const;
	bool LoadShaderObject(const void* buffer, uint size, ResourceShaderObject* shaderObject) const;

	// Shader Program (load)
	bool LoadShaderProgram(const char* programFile, ResourceShaderProgram* shaderProgram) const;
	bool LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgram* shaderProgram) const;

	void LoadDefaultShader();
	void LoadDefaultVertexShaderObject();
	void LoadDefaultFragmentShaderObject();
	void LoadDefaultShaderProgram(uint defaultVertexShaderObject, uint defaultFragmentShaderObject);

	GLuint GetDefaultVertexShaderObject() const;
	GLuint GetDefaultFragmentShaderObject() const;
	GLuint GetDefaultShaderProgram() const;

private:

	GLuint defaultVertexShaderObject = 0;
	GLuint defaultFragmentShaderObject = 0;
	GLuint defaultShaderProgram = 0;

	GLint formats = 0;
};

#endif