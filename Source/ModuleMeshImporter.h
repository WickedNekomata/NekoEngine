#ifndef __MODULE_MESH_IMPORTER_H__
#define __MODULE_MESH_IMPORTER_H__

#include "Module.h"

struct aiScene;

class ModuleMeshImporter : public Module
{
public:

	ModuleMeshImporter(bool start_enabled = true);
	~ModuleMeshImporter();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadMeshesFromFile(const char* path) const;
	bool LoadMeshesFromMemory(const char* buffer, unsigned int& bufferSize, const char* path) const;
	bool LoadMeshesWithPHYSFS(const char* path);
	void InitMeshesFromScene(const aiScene* scene, const char* path) const;
};

#endif