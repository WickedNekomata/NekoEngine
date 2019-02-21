#ifndef __RESOURCE_SHADER_PROGRAM_H__
#define __RESOURCE_SHADER_PROGRAM_H__

#include "Resource.h"

#include "ResourceShaderObject.h"

#include "Uniforms.h"

#include <list>
#include <vector>

struct ResourceShaderProgramData
{
	std::list<ResourceShaderObject*> shaderObjects;

	std::list<std::string> GetShaderObjectsNames() const
	{
		std::list<std::string> shaderObjectsNames;

		for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
			shaderObjectsNames.push_back((*it)->GetName());

		return shaderObjectsNames;
	}
};

class ResourceShaderProgram : public Resource
{
public:

	ResourceShaderProgram(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderProgramData shaderProgramData);
	~ResourceShaderProgram();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& outputFile, std::vector<std::string>& shaderObjectsNames);
	static bool ExportFile(ResourceShaderProgramData& shaderProgramData, ResourceData& data, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint shaderProgramUuid, std::string& name, std::vector<std::string>& shaderObjectsNames, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderProgramUuid, std::string& name, std::vector<std::string>& shaderObjectsNames);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	// ----------------------------------------------------------------------------------------------------

	bool Link();
	static uint Link(std::list<ResourceShaderObject*> shaderObjects);

	static uint GetBinary(uint shaderProgram, uchar** buffer);
	static uint LoadBinary(const void* buffer, int size);

	static bool DeleteShaderProgram(uint shaderProgram);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceShaderProgramData& GetSpecificData() { return shaderProgramData; }
	void SetShaderObjects(std::list<ResourceShaderObject*> shaderObjects);
	std::list<ResourceShaderObject*> GetShaderObjects(ShaderTypes shaderType = ShaderTypes::NoShaderType) const;
	std::list<std::string> GetShaderObjectsNames() const;
	void GetUniforms(std::vector<Uniform>& result) const;

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