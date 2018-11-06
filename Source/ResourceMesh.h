#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

#include "MathGeoLib/include/Math/float3.h"

struct MeshImportSettings : public ImportSettings
{
	math::float3 scale = math::float3::one;
	bool useFileScale = true;

	// Post Process
	int configuration = 0;
	bool calcTangentSpace = true;
	bool genNormals = false;
	bool genSmoothNormals = true;
	bool joinIdenticalVertices = true;
	bool triangulate = true;
	bool genUVCoords = true;
	bool sortByPType = true;
	bool improveCacheLocality = true;
	bool limitBoneWeights = true;
	bool removeRedundantMaterials = true;
	bool splitLargeMeshes = true;
	bool findDegenerates = true;
	bool findInvalidData = true;
	bool findInstances = true;
	bool validateDataStructure = true;
	bool optimizeMeshes = true;
};

class ResourceMesh : public Resource
{
public:

	ResourceMesh(ResourceType type, uint uuid);
	virtual ~ResourceMesh();

private:

	virtual void OnUniqueEditor();

	virtual bool LoadInMemory();
	virtual bool UnloadFromMemory();

public:

	float* vertices = nullptr;
	uint verticesSize = 0;
	uint verticesID = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
	uint textureCoordsSize = 0;

	MeshImportSettings* importSettings = nullptr;
};

#endif