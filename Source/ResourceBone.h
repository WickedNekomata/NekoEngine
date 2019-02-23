#ifndef __RESOURCE_BONE_H__
#define __RESOURCE_BONE_H__

#include "Resource.h"

#include "MathGeoLib\include\Math\float4x4.h"

struct ResourceBoneData
{
	// TODO
};

class ResourceBone : public Resource
{
public:

	ResourceBone(ResourceTypes type, uint uuid, ResourceData data, ResourceBoneData boneData);
	~ResourceBone();

	bool LoadInMemory();
	bool UnloadFromMemory();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceBoneData& prefabData, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceBoneData& prefab_data_output);

	void OnPanelAssets();

public:

	std::string name;
	math::float4x4 offset_matrix = math::float4x4::identity;
	uint bone_weights_size = 0u;
	float* bone_weights = nullptr;
	uint* bone_weights_indices = nullptr;
	uint mesh_uid = 0u;

	ResourceBoneData boneData;
};

#endif // __RESOURCE_BONE_H__