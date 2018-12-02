#ifndef __RESOURCE_SHADER_H__
#define __RESOURCE_SHADER_H__

#include "Resource.h"

#include "glew\include\GL\glew.h"

class ResourceShader : public Resource
{
public:

	ResourceShader(ResourceType type, uint uuid);
	~ResourceShader();

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

private:

	const char* source = nullptr;
	GLuint shaderObject = 0;
};

#endif // __RESOURCE_SHADER_H__