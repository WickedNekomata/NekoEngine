#include "GameMode.h"

#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"

#include "ModuleFileSystem.h"
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

	outputFile = shaderObject->GetName();

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

	outputFile = shaderProgram->GetName();

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
		ret = shaderObject->Compile(shaderObject->source, shaderObject->shaderType);
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
	defaultVertexShaderObject = ResourceShaderObject::Compile(vShaderTemplate, ShaderType::VertexShaderType);
}

void ShaderImporter::LoadDefaultFragmentShaderObject()
{
	defaultFragmentShaderObject = ResourceShaderObject::Compile(fShaderTemplate, ShaderType::FragmentShaderType);
}

void ShaderImporter::LoadDefaultShaderProgram(uint defaultVertexShaderObject, uint defaultFragmentShaderObject)
{
	std::list<GLuint> shaderObjects;
	shaderObjects.push_back(defaultVertexShaderObject);
	shaderObjects.push_back(defaultFragmentShaderObject);

	defaultShaderProgram = ResourceShaderProgram::Link(shaderObjects);
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