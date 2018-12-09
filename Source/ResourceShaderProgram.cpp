#include "ResourceShaderProgram.h"

#include "Application.h"
#include "ShaderImporter.h"

ResourceShaderProgram::ResourceShaderProgram(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceShaderProgram::~ResourceShaderProgram()
{
	DeleteShaderProgram(shaderProgram);
}

uint ResourceShaderProgram::LoadMemory()
{
	return LoadInMemory();
}

void ResourceShaderProgram::SetShaderObjects(std::list<ResourceShaderObject*> shaderObjects)
{
	this->shaderObjects = shaderObjects;
}

std::list<ResourceShaderObject*> ResourceShaderProgram::GetShaderObjects(ShaderType shaderType) const
{
	std::list<ResourceShaderObject*> shaderObjects;

	for (std::list<ResourceShaderObject*>::const_iterator it = this->shaderObjects.begin(); it != this->shaderObjects.end(); ++it)
	{
		if (shaderType != ShaderType::NoShaderType && (*it)->shaderType == shaderType)
			shaderObjects.push_back(*it);
		else if (shaderType == ShaderType::NoShaderType)
			shaderObjects.push_back(*it);
	}

	return shaderObjects;
}

bool ResourceShaderProgram::Link()
{
	bool ret = true;

	// Create a Shader Program
	DeleteShaderProgram(shaderProgram);
	shaderProgram = glCreateProgram();

	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glAttachShader(shaderProgram, (*it)->shaderObject);

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glDetachShader(shaderProgram, (*it)->shaderObject);

	if (!IsProgramLinked())
	{
		DeleteShaderProgram(shaderProgram);
		ret = false;
	}

	return ret;
}

GLuint ResourceShaderProgram::Link(std::list<GLuint> shaderObjects)
{
	// Create a Shader Program
	GLuint shaderProgram = glCreateProgram();

	for (std::list<GLuint>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glAttachShader(shaderProgram, *it);

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	for (std::list<GLuint>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		glDetachShader(shaderProgram, *it);

	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG("Shader Program could not be linked. ERROR: %s", infoLog);

		DeleteShaderProgram(shaderProgram);
	}
	else
		CONSOLE_LOG("Successfully linked Shader Program");

	return shaderProgram;
}

// Returns the length of the binary
GLint ResourceShaderProgram::GetBinary(GLubyte** buffer)
{
	// Get the binary length
	GLint length = 0;
	glGetProgramiv(shaderProgram, GL_PROGRAM_BINARY_LENGTH, &length);

	if (length > 0)
	{
		// Get the binary code
		*buffer = new GLubyte[length];
		GLenum format = 0;
		glGetProgramBinary(shaderProgram, length, NULL, &format, *buffer);
	}

	return length;
}

bool ResourceShaderProgram::LoadBinary(const void* buffer, GLint size)
{
	bool ret = true;

	// Create a Shader Program
	DeleteShaderProgram(shaderProgram);
	shaderProgram = glCreateProgram();

	// Install the binary
	GLenum format = 0;
	glProgramBinary(shaderProgram, format, buffer, size);

	if (!IsProgramLinked())
	{
		DeleteShaderProgram(shaderProgram);
		ret = false;
	}

	return ret;
}

bool ResourceShaderProgram::DeleteShaderProgram(GLuint& shaderProgram)
{
	bool ret = false;

	if (glIsProgram(shaderProgram))
	{
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
		ret = true;
	}

	return ret;
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
	}
	else
		CONSOLE_LOG("Successfully linked Shader Program");

	return success;
}

bool ResourceShaderProgram::LoadInMemory()
{
	return App->shaderImporter->LoadShaderProgram(exportedFile.data(), this);
}