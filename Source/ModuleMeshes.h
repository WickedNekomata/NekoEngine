#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Module.h"

#include <vector>

struct aiScene;

struct Mesh
{
	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
};

class ModuleMeshes : public Module
{
public:

	ModuleMeshes(bool start_enabled = true);
	~ModuleMeshes();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadMeshesFromFile(const char* path) const;
	bool LoadMeshesFromMemory(const char* buffer, unsigned int& bufferSize) const;
	bool LoadMeshesWithPHYSFS(const char* path);
	void InitMeshesFromScene(const aiScene* scene, const char* path) const;
};

#endif