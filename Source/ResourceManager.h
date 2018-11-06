#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Globals.h"

#include "ResourceTypes.h"

#include <map>

class Resource;

class ResourceManager
{
public:

	ResourceManager();
	~ResourceManager();

	uint Find(const char* fileInAssets) const;
	uint ImportFile(const char* newFileInAssets);
	ResourceType GetResourceTypeByExtension(const char* extension);

	const Resource* GetResource(uint uid) const;
	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);	bool DestroyResource(uint uuid);	void DestroyResources();

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;
};

#endif