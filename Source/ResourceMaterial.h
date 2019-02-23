#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"

#include "Uniforms.h"

#include "MathGeoLib\include\Math\float4.h"
#include "MathGeoLib\include\Math\float4x4.h"

#include <vector>

enum TextureTypes
{
	Albedo,
	Specular,
	NormalMap
};

struct ResourceMaterialData
{
	uint shaderUuid = 0;

	uint albedoUuid = 0;
	float albedoColor[4] = { 255.0f, 255.0f, 255.0f, 255.0f };
	uint specularUuid = 0;
	float specularColor[4] = { 51.0f, 51.0f, 51.0f, 255.0f };
	float shininess = 1.0f;
	uint normalMapUuid = 0;
	
	//math::float4x4 matrix = math::float4x4::identity;
};

class ResourceMaterial : public Resource
{
public:

	ResourceMaterial(ResourceTypes type, uint uuid, ResourceData data, ResourceMaterialData materialData);
	~ResourceMaterial();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite = false);
	static bool SaveFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite = false);
	static bool LoadFile(const char* file, ResourceMaterialData& outputMaterialData);
	
	static uint CreateMeta(const char* file, uint materialUuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& materialUuid, std::string& name);
	static uint SetNameToMeta(const char* metaFile, const std::string& name);

	// ----------------------------------------------------------------------------------------------------
	
	inline ResourceMaterialData& GetSpecificData() { return materialData; }
	void SetResourceShader(uint shaderUuid);
	uint GetShaderUuid() const;
	std::vector<Uniform>& GetUniforms();
	void SetResourceTexture(uint textureUuid, TextureTypes textureType);
	uint GetTextureUuid(TextureTypes textureType) const;
	void SetColor(math::float4& color, TextureTypes textureType);
	math::float4 GetColor(TextureTypes& textureType) const;

private:

	void EditTextureMatrix(uint textureUuid);

	bool LoadInMemory();
	bool UnloadFromMemory();

public:

	ResourceMaterialData materialData;

private:

	std::vector<Uniform> uniforms;
};

#endif