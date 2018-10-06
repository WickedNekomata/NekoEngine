#ifndef __MODULE_ASSET_IMPORTER_H__
#define __MODULE_ASSET_IMPORTER_H__

#include "Module.h"

class ModuleAssetImporter : public Module
{
public:

	ModuleAssetImporter(bool start_enabled = true);
	~ModuleAssetImporter();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadFBXfromFile(const char* path) const;
	bool LoadFBXfromMemory(const char* buffer, unsigned int& bufferSize) const;
	bool LoadFBXwithPHYSFS(const char* path);

};

#endif