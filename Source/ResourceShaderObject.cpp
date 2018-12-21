#include "ResourceShaderObject.h"

#include "ModuleGui.h"
#include "PanelCodeEditor.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"

ResourceShaderObject::ResourceShaderObject(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceShaderObject::~ResourceShaderObject()
{
	RELEASE_ARRAY(source);

	DeleteShaderObject(shaderObject);
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

bool ResourceShaderObject::Compile(bool comment)
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
	DeleteShaderObject(shaderObject);
	shaderObject = glCreateShader(shader); // Creates an empty Shader Object
	glShaderSource(shaderObject, 1, &source, NULL); // Takes an array of strings and stores it into the shader

	// Compile the Shader Object
	glCompileShader(shaderObject);

	if (!IsObjectCompiled(comment))
	{
		DeleteShaderObject(shaderObject);
		ret = false;
	}

	return ret;
}

GLuint ResourceShaderObject::Compile(const char* source, ShaderType shaderType)
{
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
	GLuint shaderObject = glCreateShader(shader); // Creates an empty Shader Object
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

		// GET ERROR LINE AND ERROR TEXT AND SEND IT TO THE CODE EDITOR
		{
			int line = 0;
			char error[DEFAULT_BUF_SIZE];
			memset(error, '\0', 100);
			for (int i = 0, count = 0; i < strlen(infoLog); ++i)
			{
				if (infoLog[i] == ':')
				{
					count++;
					if (count != 2 && count != 3)
						continue;
					for (int j = i + 1, x = 1; infoLog[j] != ':'; ++j, x *= 10)
					{
						if (count == 2)
						{
							if (infoLog[j] == '?')
							{
								line = 1;
								break;
							}
							else if (infoLog[j] != '0')
							{
								if (infoLog[j] != '0')
									line = (line * x) + infoLog[j] - '0';
							}
							else
								line *= x;
						}
						else if (count == 3)
						{
							for (int j = i + 2, x = 0; infoLog[j] != '\n'; ++j, ++x)
							{
								error[x] = infoLog[j];
							}
						}
					}
				}
			}
			App->gui->panelCodeEditor->SetError(line, error);
		}

		DeleteShaderObject(shaderObject);
	}
	else
		CONSOLE_LOG("Successfully compiled Shader Object");

	return shaderObject;
}

bool ResourceShaderObject::DeleteShaderObject(GLuint& shaderObject)
{
	bool ret = false;

	if (glIsShader(shaderObject))
	{
		glDeleteShader(shaderObject);
		shaderObject = 0;
		ret = true;
	}

	return ret;
}

bool ResourceShaderObject::IsObjectCompiled(bool comment) const
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
	else if (comment)
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