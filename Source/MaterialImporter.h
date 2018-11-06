#include "GameMode.h"

#ifndef __MATERIAL_IMPORTER_H__
#define __MATERIAL_IMPORTER_H__

#include "Importer.h"

#include <vector>

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

	bool Import(const char* importFileName, const char* importPath, std::string& outputFileName);
	bool Import(const void* buffer, uint size, std::string& outputFileName);

	void GenerateMeta(Resource* resource);
	
	bool Load(const char* exportedFileName, Texture* outputTexture);
	bool Load(const void* buffer, uint size, Texture* outputTexture);
	
	bool LoadCheckers(Texture* textureResource);

	bool IsAnisotropySupported() const;
	float GetLargestSupportedAnisotropy() const;

	uint GetDevILVersion() const;

private:

	bool isAnisotropySupported = false;
	float largestSupportedAnisotropy = 0.0f;

	//MaterialImportSettings defaultImportSettings;
};

#endif