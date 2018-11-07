#include "GameMode.h"

#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#include "Assimp/include/version.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

#include <list>

struct ResourceMesh;

struct ModelImportSettings
{
	
};

struct aiScene;
struct aiNode;

class GameObject;
class ResourceMesh;

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFileName, const char* importPath, std::string& outputFileName);
	bool Import(const void* buffer, uint size, std::string& outputFileName);
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation);

	void GenerateMeta(Resource* resource);
	bool GetMeshesUUIDsFromJson(char* fileName, std::list<uint>& uuids);

	bool Load(const char* exportedFileName, ResourceMesh* outputMesh);
	bool Load(const void* buffer, uint size, ResourceMesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;
};

#endif