#ifndef __SHADER_IMPORTER_H__
#define __SHADER_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

#include "glew\include\GL\glew.h"

class Resource;
class ResourceShader;

class ShaderImporter : public Importer
{
public:

	ShaderImporter();
	~ShaderImporter();
	
	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const { return true; }
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const { return true; }

	bool GenerateMeta(Resource* resource, std::string& outputMetaFile) const;
	bool SetShaderUUIDToMeta(const char* metaFile, uint& UUID) const;
	bool GetShaderUUIDFromMeta(const char* metaFile, uint& UUID) const;

	bool Load(const char* exportedFile, ResourceShader* outputShader) const;
	bool Load(const void* buffer, uint size, ResourceShader* outputShader) const;

	GLuint LoadDefaultVertexShaderObject() const;
	GLuint LoadDefaultFragmentShaderObject() const;
	GLuint LoadDefaultShaderProgram(uint defaultVertexShaderObject, uint defaultFragmentShaderObject) const;

	void InitDefaultShaders();
	GLuint GetDefaultVertexShaderObject() const;
	GLuint GetDefaultFragmentShaderObject() const;
	GLuint GetDefaultShaderProgram() const;

private:

	GLuint defaultVertexShaderObject = 0;
	GLuint defaultFragmentShaderObject = 0;
	GLuint defaultShaderProgram = 0;
};

#endif