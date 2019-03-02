#ifndef __RESOURCE_TEXTURE_H__
#define __RESOURCE_TEXTURE_H__

#include "Resource.h"

struct ResourceTextureImportSettings
{
	enum TextureCompression 
	{ 
		DXT1, 
		DXT3, 
		DXT5 
	};

	enum TextureWrapMode 
	{ 
		REPEAT, 
		MIRRORED_REPEAT, 
		CLAMP_TO_EDGE, 
		CLAMP_TO_BORDER 
	};

	enum TextureFilterMode 
	{ 
		NEAREST, 
		LINEAR, 
		NEAREST_MIPMAP_NEAREST, 
		LINEAR_MIPMAP_NEAREST, 
		NEAREST_MIPMAP_LINEAR, 
		LINEAR_MIPMAP_LINEAR
	};

	TextureCompression compression = TextureCompression::DXT5;
	TextureWrapMode wrapS = TextureWrapMode::REPEAT;
	TextureWrapMode wrapT = TextureWrapMode::REPEAT;
	TextureFilterMode minFilter = TextureFilterMode::LINEAR_MIPMAP_LINEAR;
	TextureFilterMode magFilter = TextureFilterMode::LINEAR;

	float anisotropy = 1.0f;

	bool UseMipmap() const
	{
		return minFilter == NEAREST_MIPMAP_NEAREST	|| magFilter == NEAREST_MIPMAP_NEAREST
			|| minFilter == LINEAR_MIPMAP_NEAREST	|| magFilter == LINEAR_MIPMAP_NEAREST
			|| minFilter == NEAREST_MIPMAP_LINEAR	|| magFilter == NEAREST_MIPMAP_LINEAR
			|| minFilter == LINEAR_MIPMAP_LINEAR	|| magFilter == LINEAR_MIPMAP_LINEAR;
	}
};

struct ResourceTextureData
{
	uint width = 0;
	uint height = 0;

	ResourceTextureImportSettings textureImportSettings;
};

class ResourceTexture : public Resource
{
public:

	ResourceTexture(ResourceTypes type, uint uuid, ResourceData data, ResourceTextureData textureData);
	~ResourceTexture();

	void OnPanelAssets();

	// ----------------------------------------------------------------------------------------------------

	static bool ImportFile(const char* file, ResourceTextureImportSettings& textureImportSettings, std::string& outputFile);
	static uint CreateMeta(const char* file, ResourceTextureImportSettings& textureImportSettings, uint textureUuid, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, ResourceTextureImportSettings& textureImportSettings, uint& textureUuid);
	static uint SetTextureImportSettingsToMeta(const char* metaFile, const ResourceTextureImportSettings& textureImportSettings);

	bool GenerateLibraryFiles() const;

	// ----------------------------------------------------------------------------------------------------

	inline ResourceTextureData& GetSpecificData() { return textureData; }
	uint GetId() const;
	uint GetWidth() const;
	uint GetHeight() const;

private:

	static bool ReadTextureUuidFromMeta(const char* metaFile, uint& textureUuid);
	static bool ReadTextureImportSettingsFromMeta(const char* metaFile, ResourceTextureImportSettings& textureImportSettings);

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	uint id = 0;

	ResourceTextureData textureData;
};

#endif