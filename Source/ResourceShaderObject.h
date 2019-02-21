#ifndef __RESOURCE_SHADER_OBJECT_H__
#define __RESOURCE_SHADER_OBJECT_H__

#include "Resource.h"

#define IS_VERTEX_SHADER(extension) strcmp(extension, EXTENSION_VERTEX_SHADER_OBJECT) == 0
#define IS_FRAGMENT_SHADER(extension) strcmp(extension, EXTENSION_FRAGMENT_SHADER_OBJECT) == 0

enum ShaderTypes
{
	NoShaderType,
	VertexShaderType,
	FragmentShaderType
};

struct ResourceShaderObjectData
{
	ShaderTypes shaderType = ShaderTypes::NoShaderType;

	void SetSource(const char* source, uint size)
	{
		RELEASE_ARRAY(this->source);
		this->source = new char[size + 1];
		strcpy_s(this->source, size + 1, source);
	}

	const char* GetSource() const
	{
		return source;
	}

private:

	char* source = nullptr;
};

class ResourceShaderObject : public Resource
{
public:

	ResourceShaderObject(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderObjectData shaderObjectData);
	~ResourceShaderObject();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceShaderObjectData& shaderObjectData, ResourceData& data, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint shaderObjectUuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderObjectUuid, std::string& name);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	// ----------------------------------------------------------------------------------------------------

	bool Compile();
	static uint Compile(const char* source, ShaderTypes shaderType);

	static bool DeleteShaderObject(uint shaderObject);

	// ----------------------------------------------------------------------------------------------------

	inline ResourceShaderObjectData& GetSpecificData() { return shaderObjectData; }
	ShaderTypes GetShaderType() const;
	void SetSource(const char* source, uint size);
	const char* GetSource() const;

private:

	static bool ReadShaderObjectUuidFromMeta(const char* metaFile, uint& shaderObjectUuid);

	bool LoadInMemory();
	bool UnloadFromMemory();

	bool IsObjectCompiled() const;

public:

	bool isValid = true;
	uint shaderObject = 0;

private:

	ResourceShaderObjectData shaderObjectData;
};

#endif // __RESOURCE_SHADER_OBJECT_H__