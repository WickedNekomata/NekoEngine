#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "Module.h"

#include <string>

class MaterialImporter : public Module
{
public:

	MaterialImporter(bool start_enabled = true);
	~MaterialImporter();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadImageFromFile(const char* path) const;

	bool Import(const char* importFile, const char* path, const char* outputFile);
	bool Import(const void* buffer, uint size, const char* outputFile);
	//bool Load(const char* exportedFile, Texture* resource);
	//bool LoadCheckers(Texture* resource);
};

#endif