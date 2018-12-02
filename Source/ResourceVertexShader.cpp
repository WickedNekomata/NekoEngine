#include "ResourceVertexShader.h"

#include "glew\include\GL\glew.h"

ResourceVertexShader::ResourceVertexShader(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceVertexShader::~ResourceVertexShader() {}

bool ResourceVertexShader::LoadInMemory()
{
	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &data, NULL);
	glCompileShader(vertexShaderID);

	int success;
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[OPEN_GL_BUF_SIZE];
		glGetShaderInfoLog(vertexShaderID, OPEN_GL_BUF_SIZE, NULL, infoLog);
		CONSOLE_LOG("Vertex Shader could not be compiled. ERROR: %s", infoLog);
	}

	return success;
}

bool ResourceVertexShader::UnloadFromMemory()
{
	glDeleteShader(vertexShaderID);
	return true;
}