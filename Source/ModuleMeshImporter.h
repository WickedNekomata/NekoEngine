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

	bool LoadMeshFromFile(const char* path) const;
	bool LoadMeshFromMemory(const char* buffer, unsigned int& bufferSize) const;
	bool LoadMeshWithPHYSFS(const char* path);
	void InitMeshFromScene(const aiScene* scene, const char* path) const;
};

#endif