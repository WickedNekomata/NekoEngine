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
#define ASCIItga 1634169902
#define ASCIITGA 1095193646
#define ASCIIdae 1700881454
#define ASCIIDAE 1161905198
#define ASCIIobj 1784835886
#define ASCIIOBJ 1245859630
#define ASCIIdds 1935959086
#define ASCIIDDS 1396982830
#define ASCIIpng 1735290926
#define ASCIIPNG 1196314670
#define ASCIIjpg 1735420462
#define ASCIIJPG 1196444206
#define ASCIIvsh 1752397358 
#define ASCIIVSH 1213421102 
#define ASCIIfsh 1752393262 
#define ASCIIFSH 1213417006
#define ASCIIpsh 1752395822
#define ASCIIPSH 1213419566

struct ImportSettings;
class Resource;

class ModuleResourceManager : public Module
{
public:

	ModuleResourceManager(bool start_enabled = true);
	~ModuleResourceManager();
	bool Start();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	// Game mode
	void RecursiveImportFilesFromLibrary(const char* dir, std::string& path);
	void ImportFileFromLibrary(const char* fileInLibrary);

	void RecursiveImportFilesFromAssets(const char* dir, std::string& path);
	void RecursiveDeleteUnusedFilesFromLibrary(const char* dir, std::string& path) const;

	void RecursiveCopyShadersIntoLibrary(const char* dir, std::string& path);

	uint ImportFile(const char* fileInAssets);
	uint ImportFile(const char* fileInAssets, const char* metaFile, const char* exportedFile);

	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);

	const Resource* GetResource(uint UUID) const;
	static ResourceType GetResourceTypeByExtension(const char* extension);

	bool FindResourcesByFile(const char* fileInAssets, std::list<uint>& UUIDs) const;
	bool FindResourcesByExportedFile(const char* exportedFile, std::list<uint>& UUIDs) const;

	int SetAsUsed(uint UUID) const;
	int SetAsUnused(uint UUID) const;

	bool DestroyResource(uint UUID);
	bool DestroyResources(std::list<uint> UUIDs);
	void DestroyResources();

	bool RemoveTextureLibraryEntry(uint UUID);
	bool RemoveMeshesLibraryEntries(std::list<uint> UUIDs);

	bool DestroyResourcesAndRemoveLibraryEntries(const char* metaFile);

	bool IsAnyResourceInVram() const;

private:

	std::map<uint, Resource*> resources;
	std::vector<ImportSettings*> importsSettings;
};

#endif