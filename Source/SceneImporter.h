#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include <vector>

struct Mesh
{
	// Unique vertices
	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	// Indices
	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	// Texture Coords
	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
	uint textureCoordsSize = 0;

	void Init();
	~Mesh();
};

struct aiScene;
struct aiNode;
class GameObject;

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFileName, const char* importPath, std::string& outputFileName);
	bool Import(const void* buffer, uint size, std::string& outputFileName);
	void RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parentGO, std::string& outputFileName, bool fbxNode = false);

	bool Load(const char* exportedFileName, Mesh* outputMesh);
	bool Load(const void* buffer, uint size, Mesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;
};

#endif