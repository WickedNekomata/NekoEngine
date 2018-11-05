#include "ResourceMaterial.h"

ResourceMaterial::ResourceMaterial(ResourceType type, uint uuid) : Resource(type, uuid)
{
}

ResourceMaterial::~ResourceMaterial()
{
}

void ResourceMaterial::OnUniqueEditor()
{
}

bool ResourceMaterial::LoadInMemory()
{
	return true;
}

bool ResourceMaterial::UnloadFromMemory()
{
	return true;
}
