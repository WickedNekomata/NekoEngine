#include "GameMode.h"

#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"

#include "ModuleFileSystem.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

#include <assert.h>

ShaderImporter::ShaderImporter() 
{
	// Verify that the driver supports at least one shader binary format
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
}

ShaderImporter::~ShaderImporter()
{
	glDeleteProgram(defaultShaderProgram);
	glDeleteShader(defaultVertexShaderObject);
	glDeleteShader(defaultFragmentShaderObject);
}

bool ShaderImporter::SaveShaderObject(ResourceShaderObject* shaderObject, std::string& outputFile) const
{
	bool ret = false;

	assert(shaderObject != nullptr);

	outputFile = shaderObject->file;

	/*
	GLubyte* buffer;
	uint size = shaderProgram->GetBinary(&buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully got Binary Program '%s'", outputFile.data());
		ret = SaveShaderProgram(buffer, size, outputFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not get Binary Program '%s'", outputFile.data());
		*/
	return ret;
}

bool ShaderImporter::SaveShaderObject(const void* buffer, uint size, std::string& outputFile) const
{
	bool ret = false;

	/*
	if (App->fs->SaveInGame((char*)buffer, size, FileType::TextureFile, outputFile) > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved Binary Program '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not save Binary Program '%s'", outputFile.data());
		*/
	return ret;
}

bool ShaderImporter::SaveShaderProgram(ResourceShaderProgram* shaderProgram, std::string& outputFile) const
{
	bool ret = false;

	if (formats == 0)
		return false;

	assert(shaderProgram != nullptr);

	outputFile = shaderProgram->name;

	GLubyte* buffer;
	uint size = shaderProgram->GetBinary(&buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully got Binary Program '%s'", outputFile.data());
		ret = SaveShaderProgram(buffer, size, outputFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not get Binary Program '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(const void* buffer, uint size, std::string& outputFile) const
{
	bool ret = false;

	if (formats == 0)
		return ret;

	if (App->fs->SaveInGame((char*)buffer, size, FileType::ShaderProgramFile, outputFile) > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved Binary Program '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not save Binary Program '%s'", outputFile.data());

	return ret;
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

bool ShaderImporter::LoadShaderObject(const char* objectFile, ResourceShaderObject* shaderObject) const
{
	bool ret = false;

	assert(objectFile != nullptr && shaderObject != nullptr);

	char* buffer;
	uint size = App->fs->Load(objectFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully loaded Shader Object '%s'", objectFile);
		ret = LoadShaderObject(buffer, size, shaderObject);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not load Shader Object '%s'", objectFile);

	return ret;
}

bool ShaderImporter::LoadShaderObject(const void* buffer, uint size, ResourceShaderObject* shaderObject) const
{
	bool ret = false;

	assert(shaderObject != nullptr);

	if (buffer != nullptr && size > 0)
	{
		shaderObject->source = new char[size];
		memcpy((char*)shaderObject->source, buffer, size);
		((char*)shaderObject->source)[size] = 0;

		// Try to compile the shader object
		ret = shaderObject->Compile();
	}

	if (ret)
	{
		CONSOLE_LOG("SHADER IMPORTER: New Shader Object loaded with: size %u", size);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Shader Object with size %u could not be loaded", size);

	return ret;
}

bool ShaderImporter::LoadShaderProgram(const char* programFile, ResourceShaderProgram* shaderProgram) const
{
	bool ret = false;

	assert(programFile != nullptr && shaderProgram != nullptr);

	char* buffer;
	uint size = App->fs->Load(programFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully loaded Shader Program '%s'", programFile);
		ret = LoadShaderProgram(buffer, size, shaderProgram);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not load Shader Program '%s'", programFile);

	return ret;
}

bool ShaderImporter::LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgram* shaderProgram) const
{
	bool ret = false;

	assert(shaderProgram != nullptr);

	if (buffer != nullptr && size > 0)
		// Try to link the shader program
		ret = shaderProgram->LoadBinary(buffer, size);

	if (ret)
	{
		CONSOLE_LOG("SHADER IMPORTER: New Shader Program loaded with: size %u", size);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Shader Program with size %u could not be loaded", size);

	return ret;
}

void ShaderImporter::LoadDefaultShader()
{
	LoadDefaultVertexShaderObject();
	LoadDefaultFragmentShaderObject();
	LoadDefaultShaderProgram(defaultVertexShaderObject, defaultFragmentShaderObject);
}

void ShaderImporter::LoadDefaultVertexShaderObject()
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

	defaultVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
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
			CONSOLE_LOG("SHADER IMPORTER: Successfully compiled Default Vertex Shader Object");
	}
}

void ShaderImporter::LoadDefaultFragmentShaderObject()
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

	defaultFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
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
}

void ShaderImporter::LoadDefaultShaderProgram(uint defaultVertexShaderObject, uint defaultFragmentShaderObject)
{
	defaultShaderProgram = glCreateProgram();

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