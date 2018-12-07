#ifndef __RESOURCE_SHADER_PROGRAM_H__
#define __RESOURCE_SHADER_PROGRAM_H__

#include "Resource.h"

#include "glew\include\GL\glew.h"

#include <list>

class ResourceShaderProgram : public Resource
{
public:

	ResourceShaderProgram(ResourceType type, uint uuid);
	~ResourceShaderProgram();

	uint LoadMemory();
	uint UnloadMemory() { return 0; }

	bool AddShaderObject(GLuint shaderObject);
	bool RemoveShaderObject(GLuint shaderObject);

	bool Link();
	uint GetBinary(GLubyte** buffer);
	bool LoadBinary(const void* buffer, uint size);

	bool IsProgramValid() const;
	bool IsProgramLinked() const;

private:

	bool LoadInMemory();
	bool UnloadFromMemory() { return true; }

public:

	const char* name = nullptr;

	std::list<GLuint> shaderObjects;
	GLuint shaderProgram = 0;
};

#endif // __RESOURCE_SHADER_PROGRAM_H__