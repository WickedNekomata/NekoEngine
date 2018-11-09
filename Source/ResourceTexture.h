#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

struct TextureImportSettings : public ImportSettings
{
	/*
	int compression = IL_DXT5;

	int wrapS = GL_REPEAT;
	int wrapT = GL_REPEAT;
	int minFilter = GL_LINEAR_MIPMAP_LINEAR;
	int magFilter = GL_LINEAR;
	*/
};

class ResourceTexture : public Resource
{
public:

	ResourceTexture(ResourceType type, uint uuid);
	virtual ~ResourceTexture();

private:

	virtual void OnUniqueEditor();

	virtual bool LoadInMemory();
	virtual bool UnloadFromMemory();

private:

	uint id = 0;
	uint width = 0;
	uint height = 0;

	TextureImportSettings* importSettings = nullptr;
};

#endif