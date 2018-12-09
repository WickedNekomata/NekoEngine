#include "ResourceShaderObject.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"

ResourceShaderObject::ResourceShaderObject(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceShaderObject::~ResourceShaderObject()
{
	RELEASE_ARRAY(source);

	glDeleteShader(shaderObject);
}

uint ResourceShaderObject::LoadMemory()
{
	return LoadInMemory();
}

void ResourceShaderObject::SetSource(const char* source, uint size)
{
	RELEASE_ARRAY(this->source);
	this->source = new char[size + 1];
	strcpy_s(this->source, size + 1, source);
}

const char* ResourceShaderObject::GetSource() const
{
	return source;
}

bool ResourceShaderObject::Compile()
{
	bool ret = true;

	GLenum shader = 0;
	switch (shaderType)
	{
	case ShaderType::VertexShaderType:
		shader = GL_VERTEX_SHADER;
		break;
	case ShaderType::FragmentShaderType:
		shader = GL_FRAGMENT_SHADER;
		break;
	}

	// Create a Shader Object
	if (glIsShader(shaderObject))
		glDeleteShader(shaderObject);
	shaderObject = glCreateShader(shader); // Creates an empty Shader Object
	glShaderSource(shaderObject, 1, &source, NULL); // Takes an array of strings and stores it into the shader

	// Compile the Shader Object
	glCompileShader(shaderObject);

	if (!IsObjectCompiled())
	{
		glDeleteShader(shaderObject);
		ret = false;
	}

	return ret;
}

bool ResourceShaderObject::IsObjectCompiled() const
{
	GLint success = 0;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetShaderInfoLog(shaderObject, logSize, NULL, infoLog);

		CONSOLE_LOG("Shader Object could not be compiled. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG("Successfully compiled Shader Object");

	return success;
}

// Returns the shader type that matches the extension
ShaderType ResourceShaderObject::GetShaderTypeByExtension(const char* extension)
{
	union
	{
		char ext[4];
		uint32_t asciiValue;
	} asciiUnion;

	for (int i = 0; i < 4; ++i)
		asciiUnion.ext[i] = extension[i];

	switch (asciiUnion.asciiValue)
	{
	case ASCIIvsh: case ASCIIVSH:
		return ShaderType::VertexShaderType;
		break;
	case ASCIIfsh: case ASCIIFSH:
		return ShaderType::FragmentShaderType;
		break;
	}

	return ShaderType::NoShaderType;
}

bool ResourceShaderObject::LoadInMemory()
{
	return App->shaderImporter->LoadShaderObject(exportedFile.data(), this);
}