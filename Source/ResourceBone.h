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