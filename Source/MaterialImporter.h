#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

class Resource;
class ResourceTexture;

struct TextureImportSettings : public ImportSettings
{
	const char* metaFile = nullptr;

	enum TextureCompression { DXT1, DXT2, DXT3, DXT4, DXT5 };
	TextureCompression compression = DXT1;

	enum TextureWrapMode { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
	TextureWrapMode wrapS = REPEAT;
	TextureWrapMode wrapT = REPEAT;

	enum TextureFilterMode { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR };
	TextureFilterMode minFilter = LINEAR_MIPMAP_LINEAR;
	TextureFilterMode magFilter = LINEAR;

	float anisotropy = 1.0f;

	bool UseMipmap() const
	{
		return minFilter == NEAREST_MIPMAP_NEAREST || magFilter == NEAREST_MIPMAP_NEAREST
			|| minFilter == LINEAR_MIPMAP_NEAREST || magFilter == LINEAR_MIPMAP_NEAREST
			|| minFilter == NEAREST_MIPMAP_LINEAR || magFilter == NEAREST_MIPMAP_LINEAR
			|| minFilter == LINEAR_MIPMAP_LINEAR || magFilter == LINEAR_MIPMAP_LINEAR;
	}

	~TextureImportSettings() { RELEASE_ARRAY(metaFile); }
};

struct Texture
{
	uint id = 0;
	uint width = 0;
	uint height = 0;
};

class MaterialImporter : public Importer
{
public:

	MaterialImporter();
	~MaterialImporter();

	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const;
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const;

	bool GenerateMeta(Resource* resource, const TextureImportSettings* textureImportSettings) const;
	bool SetTextureImportSettingsToMeta(const TextureImportSettings* textureImportSettings) const;
	bool GetTextureUUIDFromMeta(const char* metaFile, uint& UUID) const;
	bool GetTextureImportSettingsFromMeta(const char* metaFile, TextureImportSettings* textureImportSettings) const;
	
	bool Load(const char* exportedFile, Texture* outputTexture, const TextureImportSettings* textureImportSettings);
	bool Load(const void* buffer, uint size, Texture* outputTexture, const TextureImportSettings* textureImportSettings);
	
	bool LoadCheckers(Texture* textureResource);

	bool IsAnisotropySupported() const;
	float GetLargestSupportedAnisotropy() const;

	uint GetDevILVersion() const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;
};

#endif