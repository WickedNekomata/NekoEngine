#ifndef __RESOURCE_VERTEX_SHADER_H__
#define __RESOURCE_VERTEX_SHADER_H__

#include "Resource.h"

class ResourceVertexShader : public Resource
{
public:

	ResourceVertexShader(ResourceType type, uint uuid);
	~ResourceVertexShader();

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	const char* data = nullptr;
	uint vertexShaderID = 0;
};

#endif // __RESOURCE_VERTEX_SHADER_H__