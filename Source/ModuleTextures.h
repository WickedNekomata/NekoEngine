#ifndef __MODULE_TEXTURES_H__
#define __MODULE_TEXTURES_H__

#include "Module.h"

class ModuleTextures : public Module
{
public:

	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(JSON_Object* jObject);
	bool Start();
	bool CleanUp();

	uint LoadImageFromFile(const char* path, bool needsMeshes = true) const;
	uint LoadCheckImage() const;

	uint CreateTextureFromPixels(int internalFormat, uint width, uint height, uint format, const void* pixels, bool checkTexture = false) const;

	uint GetCheckTextureID() const;

	// Multitexturing example textures
	uint GetMultitexturingTextureID() const;
	uint GetMultitexturingTexture2ID() const;

private:

	uint checkTextureID = 0;

	// Multitexturing example textures
	uint multitexturingTextureID = 0;
	uint multitexturingTexture2ID = 0;
};

#endif