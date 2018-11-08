#include "ResourceTexture.h"

ResourceTexture::ResourceTexture(ResourceType type, uint uuid) : Resource(type, uuid)
{
}

ResourceTexture::~ResourceTexture()
{
}

void ResourceTexture::OnUniqueEditor()
{
}

bool ResourceTexture::LoadInMemory()
{
	return true;
}

bool ResourceTexture::UnloadFromMemory()
{
	return true;
}
