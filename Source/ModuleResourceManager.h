#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"

#include <unordered_map>

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
#define ASCIIcs 7562030
#define ASCIICS 5456686

class Resource;
struct ResourceData;
enum ResourceTypes;

class ModuleResourceManager : public Module
{
public:

	ModuleResourceManager(bool start_enabled = true);
	~ModuleResourceManager();
	bool Start();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	// ----------------------------------------------------------------------------------------------------

	Resource* ImportFile(const char* file);
	Resource* ExportFile(ResourceTypes type, ResourceData& data, void* specificData, bool overwrite = false);
	Resource* CreateResource(ResourceTypes type, ResourceData& data, void* specificData, uint forcedUuid = 0);

	// ----------------------------------------------------------------------------------------------------

	uint SetAsUsed(uint uuid) const;
	uint SetAsUnused(uint uuid) const;

	bool DeleteResource(uint uuid);
	bool DeleteResources(std::vector<uint> uuids);
	bool DeleteResources();

	void RecursiveDeleteUnusedEntries(const char* dir, std::string& path);
	void RecursiveDeleteUnusedMetas(const char* dir, std::string& path);

	// ----------------------------------------------------------------------------------------------------

	Resource* GetResource(uint uuid) const;
	bool GetResourcesUuidsByFile(const char* file, std::vector<uint>& resourcesUuids) const;
	bool GetResourceUuidByExportedFile(const char* file, uint& resourceUuid) const;
	ResourceTypes GetResourceTypeByExtension(const char* extension) const;
	std::vector<Resource*> GetResourcesByType(ResourceTypes type);

private:

	std::unordered_map<uint, Resource*> resources;
};

#endif