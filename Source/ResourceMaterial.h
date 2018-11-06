#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"

struct MaterialImportSettings : public ImportSettings
{
	int compression = 0;

	int wrapS = 0;
	int wrapT = 0;
	int minFilter = 0;
	int magFilter = 0;
};

class ResourceMaterial : public Resource
{
public:

	ResourceMaterial(ResourceType type, uint uuid);
	virtual ~ResourceMaterial();

private:

	virtual void OnUniqueEditor();

	virtual bool LoadInMemory();
	virtual bool UnloadFromMemory();

private:

	uint id = 0;
	uint width = 0;
	uint height = 0;

	MaterialImportSettings* importSettings = nullptr;
};


#endif