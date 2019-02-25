#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "GameMode.h"
#include "Globals.h"

#include <string>
#include <vector>

#define CHECKERS_WIDTH 128
#define CHECKERS_HEIGHT 128

#define REPLACE_ME_WIDTH 2
#define REPLACE_ME_HEIGHT 2

class ResourceTexture;
struct ResourceTextureData;
struct ResourceMaterialData;
struct ResourceTextureImportSettings;

class MaterialImporter
{
public:

	MaterialImporter();
	~MaterialImporter();

	bool Import(const char* file, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const char* exportedFile, ResourceTextureData& textureData, uint& textureId) const;

	// ----------------------------------------------------------------------------------------------------

	void DeleteTexture(uint& name) const;

	// ----------------------------------------------------------------------------------------------------

	void SetIsAnisotropySupported(bool isAnisotropySupported);
	bool IsAnisotropySupported() const;
	void SetLargestSupportedAnisotropy(float largestSupportedAnisotropy);
	float GetLargestSupportedAnisotropy() const;

	uint GetDevILVersion() const;

	// ----------------------------------------------------------------------------------------------------

	// *****TODO*****
	void LoadCheckers();
	void LoadDefaultTexture();
	//void LoadSkyboxTexture();
	uint LoadCubemapTexture(std::vector<uint>& faces);

	uint GetCheckers() const;
	uint GetDefaultTexture() const;
	uint GetSkyboxTexture() const;
	std::vector<uint> GetSkyboxTextures() const;
	//_*****TODO*****

private:

	bool Import(const void* buffer, uint size, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid = 0) const;

	bool Load(const void* buffer, uint size, ResourceTextureData& outputTextureData, uint& textureId) const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;

	// *****TODO*****
	uint checkers = 0;
	uint defaultTexture = 0;
	uint skyboxTexture = 0;
	std::vector<uint> skyboxTextures;
	//_*****TODO*****
};

#endif