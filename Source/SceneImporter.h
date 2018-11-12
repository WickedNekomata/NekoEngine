#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

#include "MathGeoLib/include/Math/float3.h"

#include <list>

struct aiScene;
struct aiNode;

class GameObject;
class Resource;
class ResourceMesh;

struct MeshImportSettings : public ImportSettings
{
	const char* metaFile = nullptr;

	math::float3 scale = math::float3::one;
	bool useFileScale = true;

	enum MeshPostProcessConfiguration { TARGET_REALTIME_FAST, TARGET_REALTIME_QUALITY, TARGET_REALTIME_MAX_QUALITY, CUSTOM };
	MeshPostProcessConfiguration postProcessConfiguration = TARGET_REALTIME_MAX_QUALITY;

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

	~MeshImportSettings() { RELEASE_ARRAY(metaFile); }
};

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const;
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const;
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation) const;

	bool GenerateMeta(std::list<Resource*>& resources, const MeshImportSettings* meshImportSettings) const;
	bool SetMeshImportSettingsToMeta(const char* metaFile, const MeshImportSettings* meshImportSettings) const;
	bool GetMeshesUUIDsFromMeta(const char* metaFile, std::list<uint>& UUIDs) const;
	bool GetMeshImportSettingsFromMeta(const char* metaFile, MeshImportSettings* meshImportSettings) const;

	bool Load(const char* exportedFile, ResourceMesh* outputMesh);
	bool Load(const void* buffer, uint size, ResourceMesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;
};

#endif