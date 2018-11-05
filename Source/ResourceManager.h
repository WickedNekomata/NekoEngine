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

	uint Find(const char* file_in_assets) const;
	uint ImportFile(const char* new_file_in_assets);
	Resource* const Get(uint uid);
	Resource* CreateNewResource(ResourceType type, uint force_uuid = 0);	bool DestroyResource(uint uuid);	void DestroyResources();

	bool SomethingOnMemory() const;

private:

	std::map<uint, Resource*> resources;
};

#endif