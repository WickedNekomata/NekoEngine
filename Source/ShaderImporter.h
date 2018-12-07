#ifndef __SHADER_IMPORTER_H__
#define __SHADER_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

#include "glew\include\GL\glew.h"

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