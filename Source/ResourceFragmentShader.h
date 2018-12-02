#ifndef __RESOURCE_FRAGMENT_SHADER_H__
#define __RESOURCE_FRAGMENT_SHADER_H__

#include "Resource.h"

class ResourceFragmentShader : public Resource
{
public:

	ResourceFragmentShader(ResourceType type, uint uuid);
	~ResourceFragmentShader();

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	const char* data = nullptr;
	uint fragmentShaderID = 0;
};

#endif // __RESOURCE_FRAGMENT_SHADER_H__