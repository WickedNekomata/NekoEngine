#ifndef __RESOURCE_SHADER_PROGRAM_H__
#define __RESOURCE_SHADER_PROGRAM_H__

#include "Resource.h"

#include "ResourceShaderObject.h"

#include "Uniforms.h"

#include <vector>

enum ShaderProgramTypes
{
	Standard,
	Particles,
	UI,
	Custom
};

struct ResourceShaderProgramData
{
	std::vector<uint> shaderObjectsUuids;
	ShaderProgramTypes shaderProgramType = ShaderProgramTypes::Standard;
	uint format = 0;
};

class ResourceShaderProgram : public Resource
{
public:

	ResourceShaderProgram(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderProgramData shaderProgramData);
	~ResourceShaderProgram();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes& shaderProgramType, uint& format, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceShaderProgramData& shaderProgramData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram);

	static uint CreateMeta(const char* file, uint shaderProgramUuid, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes shaderProgramType, uint format, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderProgramUuid, std::string& name, std::vector<uint>& shaderObjectsUuids, ShaderProgramTypes& shaderProgramType, uint& format);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	bool GenerateLibraryFiles() const;

	// ----------------------------------------------------------------------------------------------------

	bool Link();
	static uint Link(const std::vector<uint>& shaderObjectsUuids);

	static uint GetBinary(uint shaderProgram, uchar** buffer, uint& format);
	static uint LoadBinary(const void* buffer, int size, uint format);

	static bool DeleteShaderProgram(uint& shaderProgram);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceShaderProgramData& GetSpecificData() { return shaderProgramData; }
	void SetShaderProgramType(ShaderProgramTypes shaderProgramType);
	ShaderProgramTypes GetShaderProgramType() const;
	void SetShaderObjects(const std::vector<uint>& shaderObjectsUuids);
	void GetShaderObjects(std::vector<uint>& shaderObjectsUuids, ShaderObjectTypes shaderType = ShaderObjectTypes::NoShaderObjectType) const;
	void GetUniforms(std::vector<Uniform>& uniforms);

private:
	
	bool LoadInMemory();
	bool UnloadFromMemory();

	static bool IsProgramValid(uint shaderProgram);
	static bool IsProgramLinked(uint shaderProgram);

public:

	bool isValid = true;
	uint shaderProgram = 0;

private:
	
	ResourceShaderProgramData shaderProgramData;
};

#endif // __RESOURCE_SHADER_PROGRAM_H__