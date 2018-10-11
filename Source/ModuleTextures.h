#ifndef __MODULE_TEXTURES_H__
#define __MODULE_TEXTURES_H__

#include "Module.h"

class ModuleTextures : public Module
{
public:

	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	bool LoadImageFromFile(const char* path) const;
};

#endif