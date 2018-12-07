#include "ResourceShaderProgram.h"

#include "Application.h"
#include "ShaderImporter.h"

ResourceShaderProgram::ResourceShaderProgram(ResourceType type, uint uuid) : Resource(type, uuid) 
{
	name = new char[DEFAULT_BUF_SIZE];
}

ResourceShaderProgram::~ResourceShaderProgram()
{
	RELEASE_ARRAY(name);

	glDeleteProgram(shaderProgram);
}

uint ResourceShaderProgram::LoadMemory()
{
	return LoadInMemory();
}

bool ResourceShaderProgram::AddShaderObject(GLuint shaderObject)
{
	bool ret = std::find(shaderObjects.begin(), shaderObjects.end(), shaderObject) == shaderObjects.end();

	if (ret)
		shaderObjects.push_back(shaderObject);

	return ret;
}

bool ResourceShaderProgram::RemoveShaderObject(GLuint shaderObject)
{
	bool ret = std::find(shaderObjects.begin(), shaderObjects.end(), shaderObject) != shaderObjects.end();

	if (ret)
		shaderObjects.remove(shaderObject);

	return ret;
}

bool ResourceShaderProgram::Link()
{
	// Create a Shader Program
	shaderProgram = glCreateProgram();

	for (std::list<GLuint>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glAttachShader(shaderProgram, *it);

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	for (std::list<GLuint>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glDetachShader(shaderProgram, *it);

	return IsProgramLinked();
}

uint ResourceShaderProgram::GetBinary(GLubyte** buffer)
{
	// Get the binary length
	GLint length = 0;
	glGetProgramiv(shaderProgram, GL_PROGRAM_BINARY_LENGTH, &length);

	if (length > 0)
	{
		// Get the binary code
		*buffer = new GLubyte[length];
		GLenum format = 0;
		glGetProgramBinary(shaderProgram, length, NULL, &format, (void*)buffer);
	}

	return length;
}

bool ResourceShaderProgram::LoadBinary(const void* buffer, uint size)
{
	// Create a Shader Program
	shaderProgram = glCreateProgram();

	// Install the binary
	GLenum format = 0;
	glProgramBinary(shaderProgram, format, buffer, size);

	return IsProgramLinked();
}

bool ResourceShaderProgram::IsProgramValid() const
{
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG("Shader Program is not valid. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG("Shader Program is valid");

	return success;
}

bool ResourceShaderProgram::IsProgramLinked() const
{
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG("Shader Program could not be linked. ERROR: %s", infoLog);

		glDeleteProgram(shaderProgram); // TODO
	}
	else
		CONSOLE_LOG("Successfully linked Shader Program");

	return success;
}

bool ResourceShaderProgram::LoadInMemory()
{
	return Link();
}