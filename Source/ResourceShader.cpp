#include "ResourceShader.h"

#include "Application.h"
#include "ShaderImporter.h"

ResourceShader::ResourceShader(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceShader::~ResourceShader() 
{
	glDeleteShader(shaderObject);
}

uint ResourceShader::LoadMemory()
{
	return LoadInMemory();
}

bool ResourceShader::LoadInMemory()
{
	bool ret = App->shaderImporter->Load(exportedFile.data(), this);

	if (!ret)
		return ret;

	// 1. COMPILATION

	GLenum shaderType = 0;
	switch (type)
	{
	case Vertex_Shader_Resource:
		shaderType = GL_VERTEX_SHADER;
		break;
	case Fragment_Shader_Resource:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	}

	// Create a Shader Object
	shaderObject = glCreateShader(shaderType); // Creates an empty Shader Object
	glShaderSource(shaderObject, 1, &source, NULL); // Takes an array of strings and stores it into the shader
	
	// Compile the Shader Object
	glCompileShader(shaderObject);

	GLint success = 0;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &logSize);
		
		GLchar* infoLog = new GLchar[logSize];
		glGetShaderInfoLog(shaderObject, logSize, NULL, infoLog);

		CONSOLE_LOG("Shader Object could not be compiled. ERROR: %s", infoLog);

		glDeleteShader(shaderObject); // TODO
	}
	else
		CONSOLE_LOG("Successfully compiled Shader Object");

	return success;
}