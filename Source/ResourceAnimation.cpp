#include "ResourceAnimation.h"

ResourceAnimation::ResourceAnimation(uint uid) : Resource(ResourceType::AnimationResource, uid)
{
}

ResourceAnimation::~ResourceAnimation()
{
}

bool ResourceAnimation::LoadInMemory()
{
	return true;
}

bool ResourceAnimation::UnloadFromMemory()
{
	return true;
}
