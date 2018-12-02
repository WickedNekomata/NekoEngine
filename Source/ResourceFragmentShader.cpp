#include "ResourceFragmentShader.h"

#include "glew\include\GL\glew.h"

ResourceFragmentShader::ResourceFragmentShader(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceFragmentShader::~ResourceFragmentShader() {}

bool ResourceFragmentShader::LoadInMemory()
{
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &data, NULL);
	glCompileShader(fragmentShaderID);

	int success;
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[OPEN_GL_BUF_SIZE];
		glGetShaderInfoLog(fragmentShaderID, OPEN_GL_BUF_SIZE, NULL, infoLog);
		CONSOLE_LOG("Fragment Shader could not be compiled. ERROR: %s", infoLog);
	}

	return success;
}

bool ResourceFragmentShader::UnloadFromMemory()
{
	glDeleteShader(fragmentShaderID);
	return true;
}