#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"

#include "ResourceTypes.h"

#include <map>
#include <vector>

#define MAX_ASSETS_CHECK_TIME 2.0f

#define IS_TEXTURE_RESOURCE(extension) \
strcmp(extension, ".dds") == 0 || strcmp(extension, ".DDS") == 0 \
|| strcmp(extension, ".png") == 0 || strcmp(extension, ".PNG") == 0 \
|| strcmp(extension, ".jpg") == 0 || strcmp(extension, ".JPG") == 0
#define IS_MESH_RESOURCE(extension) \
strcmp(extension, ".fbx") == 0 || strcmp(extension, ".FBX") == 0 \
|| strcmp(extension, ".obj") == 0 || strcmp(extension, ".OBJ") == 0
#define IS_META(extension) \
strcmp(extension, ".meta") == 0 || strcmp(extension, ".META") == 0

#define ASCIIfbx 4610298120
#define ASCIIFBX 46706688
#define ASCIIobj 4611198106
#define ASCIIOBJ 46796674
#define ASCIIdds 46100100115
#define ASCIIDDS 46686883
#define ASCIIpng 46112110103
#define ASCIIPNG 46807871
#define ASCIIjpg 46106112103
#define ASCIIJPG 46748071

struct ImportSettings;
class Resource;

class ModuleResourceManager : public Module
{
public:

	ModuleResourceManager();
	~ModuleResourceManager();
	bool Start();
	update_status Update();
	bool CleanUp();

	void SetAssetsCheckTime(float assetsCheckTime);
	float GetAssetsCheckTime() const;

	void RecursiveCreateResourcesFromFilesInAssets(const char* dir, std::string& path);
	bool RecursiveFindNewFileInAssets(const char* dir, std::string& newFileInAssets) const;

	uint Find(const char* fileInAssets) const;

	uint ImportFile(const char* fileInAssets, const char* metaFile = nullptr, const char* exportedFile = nullptr);

	static ResourceType GetResourceTypeByExtension(const char* extension);

	const Resource* GetResource(uint uuid) const;
	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);
	int SetAsUsed(uint uuid) const;
	int SetAsUnused(uint uuid) const;

	bool DestroyResource(uint uuid);
	void DestroyResources();

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;
	std::vector<ImportSettings*> importsSettings;

	float assetsCheckTime = 1.0f;
	float timer = 0.0f;
};

#endif