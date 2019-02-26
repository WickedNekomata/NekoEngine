#ifndef __RESOURCE_BONE_H__
#define __RESOURCE_BONE_H__

#include "Resource.h"

#include "MathGeoLib\include\Math\float4x4.h"

struct ResourceBoneData
{
	std::string name;
	math::float4x4 offset_matrix = math::float4x4::identity;
	uint bone_weights_size = 0u;
	float* bone_weights = nullptr;
	uint* bone_weights_indices = nullptr;
	uint mesh_uid = 0u;
};

class ResourceBone : public Resource
{
public:

	ResourceBone(ResourceTypes type, uint uuid, ResourceData data, ResourceBoneData boneData);
	~ResourceBone();

	bool LoadInMemory();
	bool UnloadFromMemory();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, ResourceBoneData& bone_data, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint bone_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& bone_uuid, std::string& name);
	static bool LoadFile(const char* file, ResourceBoneData& bone_data_output);

	void OnPanelAssets();

public:

	ResourceBoneData boneData;
};

#endif // __RESOURCE_BONE_H__