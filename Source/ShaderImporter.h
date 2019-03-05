#ifndef __SHADER_IMPORTER_H__
#define __SHADER_IMPORTER_H__

#include "Globals.h"

#include <string>

class Resource;
class ResourceShaderObject;
class ResourceShaderProgram;

struct ResourceData;
struct ResourceShaderObjectData;
struct ResourceShaderProgramData;

enum ShaderObjectTypes;

class ShaderImporter
{
public:

	ShaderImporter();
	~ShaderImporter();

	bool SaveShaderObject(ResourceData& data, ResourceShaderObjectData& outputShaderObjectData, std::string& outputFile, bool overwrite = false) const;
	bool SaveShaderProgram(ResourceData& data, ResourceShaderProgramData& outputShaderProgramData, std::string& outputFile, bool overwrite = false) const;
	
	bool LoadShaderObject(const char* objectFile, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const;
	bool LoadShaderProgram(const char* programFile, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const;
	
	// ----------------------------------------------------------------------------------------------------

	void SetBinaryFormats(int formats);
	int GetBinaryFormats() const;

private:

	bool SaveShaderObject(const void* buffer, uint size, ShaderObjectTypes shaderType, std::string& outputFile, bool overwrite = false) const;
	bool SaveShaderProgram(const void* buffer, uint size, std::string& outputFile, bool overwrite = false) const;
	
	bool LoadShaderObject(const void* buffer, uint size, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const;
	bool LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const;

private:

	int formats = 0;
};

#endif