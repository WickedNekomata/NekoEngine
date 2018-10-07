#ifndef __MODULE_ASSET_IMPORTER_H__
#define __MODULE_ASSET_IMPORTER_H__

#include "Module.h"

struct aiScene;

class ModuleAssetImporter : public Module
{
public:

	ModuleAssetImporter(bool start_enabled = true);
	~ModuleAssetImporter();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadMeshFromFile(const char* path) const;
	bool LoadMeshFromMemory(const char* buffer, unsigned int& bufferSize) const;
	bool LoadMeshWithPHYSFS(const char* path);
	void InitFromScene(const aiScene* scene) const;
};

#endif