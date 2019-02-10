#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

#include "MathGeoLib\include\Math\float3.h"

#include <list>
#include <map>
#include <string>

struct aiScene;
struct aiNode;
struct aiBone;

class GameObject;
class Resource;
class ResourceMesh;

struct MeshImportSettings : public ImportSettings
{
	std::string metaFile;

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
};

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const;
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings, const char* metaFile) const;
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation, std::list<uint>& UUIDs) const;

	void RecursiveProcessBones(const aiScene* scene, const aiNode* node) const;

	bool GenerateMeta(std::list<Resource*> resources, std::string& outputMetaFile, const MeshImportSettings* meshImportSettings) const;
	bool SetMeshUUIDsToMeta(const char* metaFile, std::list<uint> UUIDs) const;
	bool GetMeshesUUIDsFromMeta(const char* metaFile, std::list<uint>& UUIDs) const;
	bool SetMeshImportSettingsToMeta(const char* metaFile, const MeshImportSettings* meshImportSettings) const;
	bool GetMeshImportSettingsFromMeta(const char* metaFile, MeshImportSettings* meshImportSettings) const;

	bool Load(const char* exportedFile, ResourceMesh* outputMesh) const;
	bool Load(const void* buffer, uint size, ResourceMesh* outputMesh) const;

	void LoadCubemap(uint& VBO, uint& VAO) const;
	void LoadPrimitivePlane();

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;

	void GetDefaultPlane(uint& defaultPlaneVAO, uint& defaultPlaneIBO, uint& defaultPlaneIndicesSize) const;

private:

	uint defaultPlaneVAO = 0;
	uint defaultPlaneIBO = 0;
	uint defaultPlaneIndicesSize = 0;

	// Anim / Bones
	/*hehehehe*/
	mutable GameObject* root_bone = nullptr;
	mutable std::map<aiBone*, uint> mesh_bone;
	mutable std::map<std::string, aiBone*> bones;
	mutable std::map<const aiNode*, GameObject*> relations;
};

#endif