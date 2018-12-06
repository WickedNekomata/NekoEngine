#include "GameMode.h"

#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"

#include "ModuleFileSystem.h"
#include "ResourceShader.h"

#include <assert.h>

ShaderImporter::ShaderImporter()
{
}

ShaderImporter::~ShaderImporter() 
{
	glDeleteProgram(defaultShaderProgram);
	glDeleteShader(defaultVertexShaderObject);
	glDeleteShader(defaultFragmentShaderObject);
}

bool ShaderImporter::GenerateMeta(Resource* resource, std::string& outputMetaFile) const
{
	if (resource == nullptr)
	{
		assert(resource != nullptr);
		return false;
	}

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	int lastModTime = App->fs->GetLastModificationTime(resource->file.data());
	json_object_set_number(rootObject, "Time Created", lastModTime);
	json_object_set_number(rootObject, "UUID", resource->GetUUID());

	// Build the path of the meta file
	outputMetaFile.append(resource->file.data());
	outputMetaFile.append(EXTENSION_META);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);

	uint size = App->fs->Save(outputMetaFile.data(), buf, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG("SHADER IMPORTER: Could not save meta '%s'", outputMetaFile.data());
		return false;
	}

	RELEASE_ARRAY(buf);
	json_value_free(rootValue);

	return true;
}

bool ShaderImporter::SetShaderUUIDToMeta(const char* metaFile, uint& UUID) const
{
	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		//CONSOLE_LOG("SHADER IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SHADER IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	json_object_set_number(rootObject, "UUID", UUID);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved meta '%s' and set its UUID", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not save meta '%s' nor set its UUID", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
	json_value_free(rootValue);

	return true;
}

bool ShaderImporter::GetShaderUUIDFromMeta(const char* metaFile, uint& UUID) const
{
	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		//CONSOLE_LOG("SHADER IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SHADER IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	UUID = json_object_get_number(rootObject, "UUID");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool ShaderImporter::Load(const char* exportedFile, ResourceShader* outputShader) const
{
	bool ret = false;

	if (exportedFile == nullptr || outputShader == nullptr)
	{
		assert(exportedFile != nullptr && outputShader != nullptr);
		return ret;
	}

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully loaded Shader '%s'", exportedFile);
		ret = Load(buffer, size, outputShader);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not load Shader '%s'", exportedFile);

	return ret;
}

bool ShaderImporter::Load(const void* buffer, uint size, ResourceShader* outputShader) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || outputShader == nullptr)
	{
		assert(buffer != nullptr && size > 0 && outputShader != nullptr);
		return ret;
	}

	outputShader->source = new char[size];
	memcpy((char*)outputShader->source, buffer, size);

	ret = true;

	CONSOLE_LOG("SHADER IMPORTER: New Shader loaded with: size %u", size);

	return ret;
}

GLuint ShaderImporter::LoadDefaultVertexShaderObject() const
{
	const GLchar* vertex_shader_glsl_330_es =
		"#version 330 core\n"
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec4 normals;\n"
		"layout (location = 2) in vec4 color;\n"
		"layout (location = 3) in vec2 texCoord;\n"
		"uniform mat4 model_matrix;\n"
		"uniform mat4 view_matrix;\n"
		"uniform mat4 proj_matrix;\n"
		"out vec4 ourColor;\n"
		"out vec2 ourTexCoord;\n"
		"void main()\n"
		"{\n"
		"    ourTexCoord = texCoord;\n"
		"    ourColor = color;\n"
		"    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);\n"
		"}\n";

	GLuint defaultVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(defaultVertexShaderObject, 1, &vertex_shader_glsl_330_es, NULL);

	glCompileShader(defaultVertexShaderObject);
	{
		GLint success = 0;
		glGetShaderiv(defaultVertexShaderObject, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(defaultVertexShaderObject, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetShaderInfoLog(defaultVertexShaderObject, logSize, NULL, infoLog);

			CONSOLE_LOG("SHADER IMPORTER: Default Vertex Shader Object could not be compiled. ERROR: %s", infoLog);

			glDeleteShader(defaultVertexShaderObject);
		}
		else
			CONSOLE_LOG("Successfully compiled Default Vertex Shader Object");
	}

	return defaultVertexShaderObject;
}

GLuint ShaderImporter::LoadDefaultFragmentShaderObject() const
{
	const GLchar* fragment_shader_glsl_330_es =
		"#version 330 core\n"
		"in vec4 ourColor;\n"
		"in vec2 ourTexCoord;\n"
		"out vec4 FragColor;\n"
		"uniform sampler2D ourTexture_0;\n"
		"void main()\n"
		"{\n"
		"     FragColor = texture(ourTexture_0, ourTexCoord);\n"
		"}\n";

	GLuint defaultFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(defaultFragmentShaderObject, 1, &fragment_shader_glsl_330_es, NULL);

	glCompileShader(defaultFragmentShaderObject);
	{
		GLint success = 0;
		glGetShaderiv(defaultFragmentShaderObject, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(defaultFragmentShaderObject, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetShaderInfoLog(defaultFragmentShaderObject, logSize, NULL, infoLog);

			CONSOLE_LOG("SHADER IMPORTER: Default Fragment Shader Object could not be compiled. ERROR: %s", infoLog);

			glDeleteShader(defaultFragmentShaderObject);
		}
		else
			CONSOLE_LOG("SHADER IMPORTER: Successfully compiled Default Fragment Shader Object");
	}

	return defaultFragmentShaderObject;
}

GLuint ShaderImporter::LoadDefaultShaderProgram(uint defaultVertexShaderObject, uint defaultFragmentShaderObject) const
{
	GLuint defaultShaderProgram = glCreateProgram();

	glAttachShader(defaultShaderProgram, defaultVertexShaderObject);
	glAttachShader(defaultShaderProgram, defaultFragmentShaderObject);

	glLinkProgram(defaultShaderProgram);

	glDetachShader(defaultShaderProgram, defaultVertexShaderObject);
	glDetachShader(defaultShaderProgram, defaultFragmentShaderObject);

	{
		GLint success;
		glGetProgramiv(defaultShaderProgram, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(defaultShaderProgram, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetProgramInfoLog(defaultShaderProgram, logSize, NULL, infoLog);

			CONSOLE_LOG("SHADER IMPORTER: Default Shader Program could not be linked. ERROR: %s", infoLog);

			glDeleteProgram(defaultShaderProgram);
		}
		else
			CONSOLE_LOG("SHADER IMPORTER: Successfully linked Default Shader Program");
	}

	return defaultShaderProgram;
}

void ShaderImporter::InitDefaultShaders()
{
	defaultVertexShaderObject = LoadDefaultVertexShaderObject();
	defaultFragmentShaderObject = LoadDefaultFragmentShaderObject();
	defaultShaderProgram = LoadDefaultShaderProgram(defaultVertexShaderObject, defaultFragmentShaderObject);
}

GLuint ShaderImporter::GetDefaultVertexShaderObject() const
{
	return defaultVertexShaderObject;
}

GLuint ShaderImporter::GetDefaultFragmentShaderObject() const
{
	return defaultFragmentShaderObject;
}

GLuint ShaderImporter::GetDefaultShaderProgram() const
{
	return defaultShaderProgram;
}