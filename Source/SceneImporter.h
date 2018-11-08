#include "GameMode.h"

#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include <list>

struct aiScene;
struct aiNode;

class GameObject;
class ResourceMesh;

struct MeshImportSettings;

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFileName, const char* importPath, std::string& outputFileName, const ImportSettings* importSettings);
	bool Import(const void* buffer, uint size, std::string& outputFileName, const ImportSettings* importSettings);
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation);

	void GenerateMeta(std::list<Resource*>& meshResources) const;
	bool GetMeshesUUIDsFromMeta(const char* metaFile, std::list<uint>& UUIDs) const;
	bool GetMeshImportSettingsFromMeta(const char* metaFile, MeshImportSettings* meshImportSettings) const;

	bool Load(const char* exportedFileName, ResourceMesh* outputMesh);
	bool Load(const void* buffer, uint size, ResourceMesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;
};

#endif