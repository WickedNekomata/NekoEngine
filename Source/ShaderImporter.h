#ifndef __SHADER_IMPORTER_H__
#define __SHADER_IMPORTER_H__

#include "GameMode.h"
#include "Globals.h"

#include <string>

class Resource;
class ResourceShaderObject;
class ResourceShaderProgram;

struct ResourceData;
struct ResourceShaderObjectData;
struct ResourceShaderProgramData;

enum ShaderTypes;

class ShaderImporter
{
public:

	ShaderImporter();
	~ShaderImporter();

	bool CreateShaderObject(std::string& file, ShaderTypes shaderType) const;

	// Shader Object (save)
	bool SaveShaderObject(ResourceData& data, ResourceShaderObjectData& outputShaderObjectData, std::string& outputFile, bool overwrite = false) const;
	bool SaveShaderObject(const void* buffer, uint size, ShaderTypes shaderType, std::string& outputFile, bool overwrite = false) const;

	// Shader Program (save)
	bool SaveShaderProgram(ResourceData& data, ResourceShaderProgramData& outputShaderProgramData, std::string& outputFile, bool overwrite = false) const;
	bool SaveShaderProgram(const void* buffer, uint size, std::string& outputFile, bool overwrite = false) const;

	// Shader Object (load)
	bool LoadShaderObject(const char* objectFile, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const;
	bool LoadShaderObject(const void* buffer, uint size, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const;

	// Shader Program (load)
	bool LoadShaderProgram(const char* programFile, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const;
	bool LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const;
	
	// ----------------------------------------------------------------------------------------------------

	void SetBinaryFormats(int formats);
	int GetBinaryFormats() const;

	// ----------------------------------------------------------------------------------------------------

	// *****TODO*****
	void LoadDefaultShader();
	void LoadCubemapShader();
	uint LoadDefaultShaderObject(ShaderTypes shaderType) const;
	uint LoadShaderProgram(uint vertexShaderObject, uint fragmentShaderObject) const;

	uint GetDefaultVertexShaderObject() const;
	uint GetDefaultFragmentShaderObject() const;
	uint GetDefaultShaderProgram() const;
	uint GetCubemapShaderProgram() const;
	//_*****TODO*****

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
