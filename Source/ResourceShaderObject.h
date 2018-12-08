#ifndef __RESOURCE_SHADER_OBJECT_H__
#define __RESOURCE_SHADER_OBJECT_H__

#include "Resource.h"

#include "glew\include\GL\glew.h"

enum ShaderType
{
	NoShaderType,
	VertexShaderType,
	FragmentShaderType
};

class ResourceShaderObject : public Resource
{
public:

	ResourceShaderObject(ResourceType type, uint uuid);
	~ResourceShaderObject();

	uint LoadMemory();
	uint UnloadMemory() { return 0; }

	void SetSource(const char* source, uint size);
	const char* GetSource() const;

	static GLuint Compile(const char* source, ShaderType shaderType);

	static bool IsObjectCompiled(GLuint shaderObject);

	static bool DeleteShaderObject(GLuint shaderObject);

	static ShaderType GetShaderTypeByExtension(const char* extension);

private:

	bool LoadInMemory();
	bool UnloadFromMemory() { return true; }

public:

	ShaderType shaderType = ShaderType::NoShaderType;
	GLuint shaderObject = 0;

private:

	const char* source = nullptr;
};

#endif // __RESOURCE_SHADER_OBJECT_H__