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
<<<<<<< HEAD:Source/ModuleAssets.h
	void InitFromScene(const aiScene* scene, const char* path) const;
=======
	void InitMeshFromScene(const aiScene* scene) const;
>>>>>>> 039bfbe32d6542b0313c7b9ec91192dbe76d1a4d:Source/ModuleMeshImporter.h
};

#endif