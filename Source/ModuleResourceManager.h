#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"

#include "ResourceTypes.h"

#include <map>
#include <vector>

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
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void RecursiveImportFilesFromDir(const char* dir, std::string& path);
	void RecursiveDeleteUnusedFilesFromDir(const char* dir, std::string& path);

	uint ImportFile(const char* fileInAssets);
	uint ImportFile(const char* fileInAssets, const char* metaFile, const char* exportedFile);

	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);

	const Resource* GetResource(uint uuid) const;
	static ResourceType GetResourceTypeByExtension(const char* extension);
	uint FindByFile(const char* fileInAssets) const;
	uint FindByExportedFile(const char* exportedFile) const;

	int SetAsUsed(uint uuid) const;
	int SetAsUnused(uint uuid) const;

	bool DestroyResource(uint uuid);
	void DestroyResources();

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;
	std::vector<ImportSettings*> importsSettings;
};

#endif