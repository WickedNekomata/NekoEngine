#ifndef __RESOURCE_SHADER_PROGRAM_H__
#define __RESOURCE_SHADER_PROGRAM_H__

#include "Resource.h"

#include "ResourceShaderObject.h"

#include "Uniforms.h"

#include <list>
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
	ShaderProgramTypes shaderProgramType = ShaderProgramTypes::Standard;

	uint format = 0;

	std::list<ResourceShaderObject*> shaderObjects;

	std::list<std::string> GetShaderObjectsNames() const;	
};

class ResourceShaderProgram : public Resource
{
public:

	ResourceShaderProgram(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderProgramData shaderProgramData);
	~ResourceShaderProgram();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::vector<std::string>& shaderObjectsNames, ShaderProgramTypes& shaderProgramType, uint& format, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceShaderProgramData& shaderProgramData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram);

	static uint CreateMeta(const char* file, uint shaderProgramUuid, std::string& name, std::vector<std::string>& shaderObjectsNames, ShaderProgramTypes shaderProgramType, uint format, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderProgramUuid, std::string& name, std::vector<std::string>& shaderObjectsNames, ShaderProgramTypes& shaderProgramType, uint& format);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	// ----------------------------------------------------------------------------------------------------

	bool Link();
	static uint Link(std::list<ResourceShaderObject*> shaderObjects);

	static uint GetBinary(uint shaderProgram, uchar** buffer, uint& format);
	static uint LoadBinary(const void* buffer, int size, uint format);

	static bool DeleteShaderProgram(uint& shaderProgram);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceShaderProgramData& GetSpecificData() { return shaderProgramData; }
	void SetShaderProgramType(ShaderProgramTypes shaderProgramType);
	ShaderProgramTypes GetShaderProgramType() const;
	void SetShaderObjects(std::list<ResourceShaderObject*> shaderObjects);
	std::list<ResourceShaderObject*> GetShaderObjects(ShaderObjectTypes shaderType = ShaderObjectTypes::NoShaderObjectType) const;
	std::list<std::string> GetShaderObjectsNames() const;
	void GetUniforms(std::vector<Uniform>& result);

private:

	static bool ReadShaderProgramUuidFromMeta(const char* metaFile, uint& shaderProgramUuid);
	static bool ReadShaderObjectsNamesFromMeta(const char* metaFile, std::vector<std::string>& shaderObjectsNames);

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