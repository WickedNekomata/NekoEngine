#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "Globals.h"

#include <string>
#include <vector>

#define CHECKERS_WIDTH 128
#define CHECKERS_HEIGHT 128

#define REPLACE_ME_WIDTH 2
#define REPLACE_ME_HEIGHT 2

struct ResourceData;
struct ResourceTextureData;
struct ResourceTextureImportSettings;

class MaterialImporter
{
public:

	MaterialImporter();
	~MaterialImporter();

	bool Import(const char* file, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const char* exportedFile, ResourceData& outputData, ResourceTextureData& textureData) const;

	// ----------------------------------------------------------------------------------------------------

	void LoadInMemory(uint& id, const ResourceTextureData& textureData);

	void DeleteTexture(uint& name) const;

	// ----------------------------------------------------------------------------------------------------

	void SetIsAnisotropySupported(bool isAnisotropySupported);
	bool IsAnisotropySupported() const;
	void SetLargestSupportedAnisotropy(float largestSupportedAnisotropy);
	float GetLargestSupportedAnisotropy() const;

	uint GetDevILVersion() const;

private:

	bool Import(const void* buffer, uint size, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const void* buffer, uint size, ResourceData& outputData, ResourceTextureData& outputTextureData) const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;
};

#endif