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

	// ----------------------------------------------------------------------------------------------------

	// *****TODO*****
	void LoadDefaultShader();
	void LoadCubemapShader();
	uint LoadDefaultShaderObject(ShaderObjectTypes shaderType) const;
	uint LoadShaderProgram(uint vertexShaderObject, uint fragmentShaderObject) const;

	uint GetDefaultVertexShaderObject() const;
	uint GetDefaultFragmentShaderObject() const;
	uint GetDefaultShaderProgram() const;
	uint GetCubemapShaderProgram() const;
	//_*****TODO*****

private:

	bool SaveShaderObject(const void* buffer, uint size, ShaderObjectTypes shaderType, std::string& outputFile, bool overwrite = false) const;
	bool SaveShaderProgram(const void* buffer, uint size, std::string& outputFile, bool overwrite = false) const;
	
	bool LoadShaderObject(const void* buffer, uint size, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const;
	bool LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const;

private:

	int formats = 0;

	// *****TODO*****
	uint defaultVertexShaderObject = 0;
	uint defaultFragmentShaderObject = 0;
	uint defaultShaderProgram = 0;

	uint cubemapShaderProgram = 0;
	//_*****TODO*****
};

#endif