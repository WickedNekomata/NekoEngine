#ifndef __MODULE_ASSETS_H__
#define __MODULE_ASSETS_H__

#include "Module.h"

struct aiScene;

class ModuleAssets : public Module
{
public:

	ModuleAssets(bool start_enabled = true);
	~ModuleAssets();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadMeshFromFile(const char* path) const;
	bool LoadMeshFromMemory(const char* buffer, unsigned int& bufferSize) const;
	bool LoadMeshWithPHYSFS(const char* path);
	void InitFromScene(const aiScene* scene, const char* path) const;
};

#endif