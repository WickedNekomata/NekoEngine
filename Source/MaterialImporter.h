#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "Importer.h"
#include "GameMode.h"

class Resource;
class ResourceTexture;

struct TextureImportSettings : public ImportSettings
{
	const char* metaFile = nullptr;

	enum TextureCompression { DXT1, DXT3, DXT5 };
	TextureCompression compression = DXT5;

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

class MaterialImporter : public Importer
{
public:

	MaterialImporter();
	~MaterialImporter();

	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const;
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const;

	bool GenerateMeta(Resource* resource, std::string& outputMetaFile, const TextureImportSettings* textureImportSettings) const;
	bool SetTextureUUIDToMeta(const char* metaFile, uint& UUID) const;
	bool SetTextureImportSettingsToMeta(const char* metaFile, const TextureImportSettings* textureImportSettings) const;
	bool GetTextureUUIDFromMeta(const char* metaFile, uint& UUID) const;
	bool GetTextureImportSettingsFromMeta(const char* metaFile, TextureImportSettings* textureImportSettings) const;
	
	bool Load(const char* exportedFile, ResourceTexture* outputTexture, const TextureImportSettings* textureImportSettings) const;
	bool Load(const void* buffer, uint size, ResourceTexture* outputTexture, const TextureImportSettings* textureImportSettings) const;
	
	void LoadCheckers();
	void LoadDefaultTexture();

	uint GetCheckers() const;
	uint GetDefaultTexture() const;

	bool IsAnisotropySupported() const;
	float GetLargestSupportedAnisotropy() const;

	uint GetDevILVersion() const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;

	uint checkers = 0;
	uint defaultTexture = 0;
};

#endif