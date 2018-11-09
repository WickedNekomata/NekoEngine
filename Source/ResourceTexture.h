#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

struct TextureImportSettings : public ImportSettings
{
	enum TextureCompression { DXT1, DXT2, DXT3, DXT4, DXT5 };
	TextureCompression compression = DXT1;

	enum TextureWrapMode { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
	TextureWrapMode wrapS = REPEAT;
	TextureWrapMode wrapT = REPEAT;

	enum TextureFilterMode { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR };
	TextureFilterMode minFilter = LINEAR_MIPMAP_LINEAR;
	TextureFilterMode magFilter = LINEAR;
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