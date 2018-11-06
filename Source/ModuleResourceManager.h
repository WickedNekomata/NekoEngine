#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"

#include "ResourceTypes.h"

#include <map>

#define MAX_ASSETS_CHECK_TIME 2.0f

class Resource;

class ModuleResourceManager : public Module
{
public:

	ModuleResourceManager();
	~ModuleResourceManager();
	update_status Update();
	bool CleanUp();

	void SetAssetsCheckTime(float assetsCheckTime);
	float GetAssetsCheckTime() const;

	uint Find(const char* fileInAssets) const;
	uint ImportFile(const char* newFileInAssets);
	ResourceType GetResourceTypeByExtension(const char* extension);

	const Resource* GetResource(uint uuid) const;
	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);	int SetAsUsed(uint uuid) const;	int SetAsUnused(uint uuid) const;	bool DestroyResource(uint uuid);	void DestroyResources();

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;

	float assetsCheckTime = 1.0f;
	float timer = 0.0f;
};

#endif