#include "ResourceBone.h"

ResourceBone::ResourceBone(uint uid) : Resource(ResourceType::BoneResource, uid)
{
}

ResourceBone::~ResourceBone()
{
}

bool ResourceBone::LoadInMemory()
{
	return true;
}

bool ResourceBone::UnloadFromMemory()
{
	return true;
}
