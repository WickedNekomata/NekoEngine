#ifndef __RESOURCE_SHADER_PROGRAM_H__
#define __RESOURCE_SHADER_PROGRAM_H__

#include "Resource.h"

#include "ResourceShaderObject.h"

#include "Uniforms.h"

#include "glew\include\GL\glew.h"

#include <list>
#include <vector>

class ResourceShaderProgram : public Resource
{
public:

	ResourceShaderProgram(ResourceType type, uint uuid);
	~ResourceShaderProgram();

	uint LoadMemory();
	uint UnloadMemory() { return 0; }

	void SetShaderObjects(std::list<ResourceShaderObject*> shaderObjects);
	std::list<ResourceShaderObject*> GetShaderObjects(ShaderType shaderType = ShaderType::NoShaderType) const;

	bool Link(bool comment = true);
	static GLuint Link(std::list<GLuint> shaderObjects);

	GLint GetBinary(GLubyte** buffer);
	bool LoadBinary(const void* buffer, GLint size);

	static bool DeleteShaderProgram(GLuint& shaderProgram);

	bool IsProgramValid() const;
	bool IsProgramLinked(bool comment = true) const;

	void GetUniforms(std::vector<Uniform*>& result) const;

private:

	bool LoadInMemory();
	bool UnloadFromMemory() { return true; }

private:

	std::list<ResourceShaderObject*>	shaderObjects;

public:

	GLuint shaderProgram = 0;
};

#endif // __RESOURCE_SHADER_PROGRAM_H__