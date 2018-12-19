#include "ResourceShaderProgram.h"

#include "Application.h"
#include "ShaderImporter.h"

#include <assert.h>

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

bool ResourceShaderProgram::Link(bool comment)
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

	if (!IsProgramLinked(comment))
	{
		DeleteShaderProgram(shaderProgram);
		ret = false;
	}
	else
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::ShaderProgramChanged;
		newEvent.shaderEvent.shader = shaderProgram;
		App->PushSystemEvent(newEvent);
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

bool ResourceShaderProgram::IsProgramLinked(bool comment) const
{
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		if (comment)
			CONSOLE_LOG("Shader Program could not be linked. ERROR: %s", infoLog);
	}
	else if (comment)
		CONSOLE_LOG("Successfully linked Shader Program");

	return success;
}

void ResourceShaderProgram::GetUniforms(std::vector<Uniform*>& uniforms) const
{
	int count;
	glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
	uniforms.reserve(count);

	GLuint program;
	GLuint index;
	GLsizei length;
	GLint size;
	GLenum type;
	GLchar name[DEFAULT_BUF_SIZE];

	for (int i = 0; i < count; ++i)
	{
		glGetActiveUniform(shaderProgram, (GLuint)i, DEFAULT_BUF_SIZE, &length, &size, &type, name);

		if (strcmp(name, "model_matrix") == 0 || strcmp(name, "view_matrix") == 0 || strcmp(name, "proj_matrix") == 0
			|| strcmp(name, "light.direction") == 0 || strcmp(name, "light.ambient") == 0 || strcmp(name, "light.diffuse") == 0 || strcmp(name, "light.specular") == 0
			|| strcmp(name, "Time") == 0 || strcmp(name, "viewPos") == 0)
			continue;

		Uniform* uniform;
		switch (type)
		{
		case Uniforms_Values::FloatU_value:
			uniform = new Uniform();
			strcpy_s(uniform->floatU.name, name);
			uniform->floatU.type = type;
			uniform->floatU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::IntU_value:
			uniform = new Uniform();
			strcpy_s(uniform->intU.name, name);
			uniform->intU.type = type;
			uniform->intU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec2FU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec2FU.name, name);
			uniform->vec2FU.type = type;
			uniform->vec2FU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec3FU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec3FU.name, name);
			uniform->vec3FU.type = type;
			uniform->vec3FU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec4FU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec4FU.name, name);
			uniform->vec4FU.type = type;
			uniform->vec4FU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec2IU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec2IU.name, name);
			uniform->vec2IU.type = type;
			uniform->vec2IU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec3IU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec3IU.name, name);
			uniform->vec3IU.type = type;
			uniform->vec3IU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		case Uniforms_Values::Vec4IU_value:
			uniform = new Uniform();
			strcpy_s(uniform->vec4IU.name, name);
			uniform->vec4IU.type = type;
			uniform->vec4IU.location = glGetUniformLocation(shaderProgram, uniform->common.name);
			break;
		default:
			continue;
			break;
		}
		uniforms.push_back(uniform);
	}
	uniforms.shrink_to_fit();
}

bool ResourceShaderProgram::LoadInMemory()
{
	return App->shaderImporter->LoadShaderProgram(exportedFile.data(), this);
}