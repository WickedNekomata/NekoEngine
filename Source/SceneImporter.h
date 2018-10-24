#ifndef __SCENE_IMPORTER_H__
#define __SCENE_IMPORTER_H__

#include "Importer.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include <vector>

struct aiScene;

struct Mesh
{
	const char* name = nullptr;

	math::float3 position;
	math::float3 scale;
	math::Quat rotation;

	// Unique vertices
	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	// Indices
	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	// Normals
	//float* normals = nullptr;
	//PrimitiveRay** normalsVerticesDebug = nullptr;
	//PrimitiveRay** normalsFacesDebug = nullptr;

	// Texture Coords
	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
	uint textureCoordsSize = 0;
};

class SceneImporter : public Importer
{
public:

	SceneImporter();
	~SceneImporter();

	bool Import(const char* importFile, const char* importPath, std::string& outputFile);
	bool Import(const void* buffer, uint size, std::string& outputFile);

	bool Load(const char* exportedFile, Mesh* outputMesh);
	bool Load(const void* buffer, uint size, Mesh* outputMesh);

	uint GetAssimpMajorVersion() const;
	uint GetAssimpMinorVersion() const;
	uint GetAssimpRevisionVersion() const;
};

#endif