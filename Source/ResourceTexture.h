#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

struct TextureImportSettings : public ImportSettings
{
	int compression = 0;

	int wrapS = 0;
	int wrapT = 0;
	int minFilter = 0;
	int magFilter = 0;
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