#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"

#include "ResourceTypes.h"

#include <map>
#include <vector>
#include <list>

// This is the value in memory of each string.
// https://www.quora.com/How-is-an-integer-value-stored-in-the-memory-in-C
#define ASCIIfbx 2019714606
#define ASCIIFBX 1480738350
#define ASCIIobj 1784835886
#define ASCIIOBJ 1245859630
#define ASCIIdds 1935959086
#define ASCIIDDS 1396982830
#define ASCIIpng 1735290926
#define ASCIIPNG 1196314670
#define ASCIIjpg 1735420462
#define ASCIIJPG 1196444206

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

	const Resource* GetResource(uint UUID) const;
	static ResourceType GetResourceTypeByExtension(const char* extension);

	bool FindTextureByFile(const char* fileInAssets, uint& UUID) const;
	bool FindTextureByExportedFile(const char* exportedFile, uint& UUID) const;
	bool FindMeshesByFile(const char* fileInAssets, std::list<uint>& UUIDs) const;
	bool FindMeshesByExportedFile(const char* exportedFile, std::list<uint>& UUIDs) const;

	int SetAsUsed(uint UUID) const;
	int SetAsUnused(uint UUID) const;

	bool DestroyResource(uint UUID);
	bool DestroyResources(std::list<uint> UUIDs);
	void DestroyResources();

	bool RemoveTextureLibraryEntry(uint UUID);
	bool RemoveMeshesLibraryEntries(std::list<uint> UUIDs);

	bool DestroyResourcesAndRemoveLibraryEntries(const char* metaFile);

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;
	std::vector<ImportSettings*> importsSettings;
};

#endif