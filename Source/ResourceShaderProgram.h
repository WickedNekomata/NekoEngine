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

	static GLuint Link(std::list<GLuint> shaderObjects);

	static GLint GetBinary(GLint shaderProgram, GLubyte** buffer);
	GLuint LoadBinary(const void* buffer, GLint size);

	static bool IsProgramValid(GLuint shaderProgram);
	static bool IsProgramLinked(GLuint shaderProgram);

private:

	bool LoadInMemory();
	bool UnloadFromMemory() { return true; }

public:

	std::list<GLuint> shaderObjects;
	GLuint shaderProgram = 0;
};

#endif // __RESOURCE_SHADER_PROGRAM_H__