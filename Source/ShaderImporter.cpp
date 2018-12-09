#include "GameMode.h"

#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"

#include "ModuleFileSystem.h"
#include "ResourceShaderProgram.h"

#include <assert.h>

ShaderImporter::ShaderImporter() {}

ShaderImporter::~ShaderImporter()
{
	glDeleteProgram(cubemapShaderProgram);
	glDeleteProgram(defaultShaderProgram);
	glDeleteShader(defaultVertexShaderObject);
	glDeleteShader(defaultFragmentShaderObject);
}

bool ShaderImporter::SaveShaderObject(const char* name, const char* file, const char* buffer, ShaderType shaderType, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	if (overwrite)
		outputFile = file;
	else
		outputFile = name;

	uint size = 0;
	if (buffer != nullptr)
		size = strlen(buffer);

	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully read Shader Object '%s'", name);
		ret = SaveShaderObject(buffer, size, shaderType, outputFile, overwrite);
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not read Shader Object '%s'", name);

	return ret;
}

bool ShaderImporter::SaveShaderObject(const void* buffer, uint size, ShaderType shaderType, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	FileType fileType = FileType::NoFileType;
	switch (shaderType)
	{
	case ShaderType::VertexShaderType:
		fileType = FileType::VertexShaderObjectFile;
		break;
	case ShaderType::FragmentShaderType:
		fileType = FileType::FragmentShaderObjectFile;
		break;
	}

	if (App->fs->SaveInGame((char*)buffer, size, fileType, outputFile, overwrite) > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved Shader Object '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG("SHADER IMPORTER: Could not save Shader Object '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(const char* name, GLuint shaderProgram, std::string& outputFile) const
{
	bool ret = false;

	// Verify that the driver supports at least one shader binary format
	if (GetBinaryFormats() == 0)
		return ret;

	outputFile = name;

	GLubyte* buffer;
	uint size = ResourceShaderProgram::GetBinary(shaderProgram, &buffer);
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

	// Verify that the driver supports at least one shader binary format
	if (GetBinaryFormats() == 0)
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

bool ShaderImporter::GenerateShaderObjectMeta(ResourceShaderObject* shaderObject, std::string& outputMetaFile) const
{
	if (shaderObject == nullptr)
	{
		assert(shaderObject != nullptr);
		return false;
	}

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	int lastModTime = App->fs->GetLastModificationTime(shaderObject->file.data());
	json_object_set_number(rootObject, "Time Created", lastModTime);
	json_object_set_number(rootObject, "UUID", shaderObject->GetUUID());

	// Build the path of the meta file
	outputMetaFile.append(shaderObject->file.data());
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

bool ShaderImporter::GenerateShaderProgramMeta(ResourceShaderProgram* shaderProgram, std::string& outputMetaFile) const
{
	if (shaderProgram == nullptr)
	{
		assert(shaderProgram != nullptr);
		return false;
	}

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	int lastModTime = App->fs->GetLastModificationTime(shaderProgram->file.data());
	json_object_set_number(rootObject, "Time Created", lastModTime);
	json_object_set_number(rootObject, "UUID", shaderProgram->GetUUID());

	JSON_Value* shaderObjectsArrayValue = json_value_init_array();
	JSON_Array* shaderObjectsArray = json_value_get_array(shaderObjectsArrayValue);
	std::list<ResourceShaderObject*> shaderObjects = shaderProgram->shaderObjects;
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
	{
		std::string fileName;
		App->fs->GetFileName((*it)->file.data(), fileName, true);
		json_array_append_string(shaderObjectsArray, fileName.data());
	}
	json_object_set_value(rootObject, "Shader Objects", shaderObjectsArrayValue);

	// Build the path of the meta file
	outputMetaFile.append(shaderProgram->file.data());
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

bool ShaderImporter::SetShaderUUIDToMeta(const char* metaFile, uint UUID) const
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
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved meta '%s' and set its UUID", metaFile);
	}
	else
	{
		CONSOLE_LOG("SHADER IMPORTER: Could not save meta '%s' nor set its UUID", metaFile);
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

bool ShaderImporter::SetShaderObjectsToMeta(const char* metaFile, std::list<ResourceShaderObject*> shaderObjects) const
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

	JSON_Value* shaderObjectsArrayValue = json_value_init_array();
	JSON_Array* shaderObjectsArray = json_value_get_array(shaderObjectsArrayValue);
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
	{
		std::string fileName;
		App->fs->GetFileName((*it)->file.data(), fileName, true);
		json_array_append_string(shaderObjectsArray, fileName.data());
	}
	json_object_set_value(rootObject, "Shader Objects", shaderObjectsArrayValue);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("SHADER IMPORTER: Successfully saved meta '%s' and set its Shader Objects UUIDs", metaFile);
	}
	else
	{
		CONSOLE_LOG("SHADER IMPORTER: Could not save meta '%s' nor set its Shader Objects UUIDs", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
	json_value_free(rootValue);

	return true;
}

bool ShaderImporter::GetShaderObjectsFromMeta(const char* metaFile, std::list<std::string>& files) const
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

	JSON_Array* shaderObjectsArray = json_object_get_array(rootObject, "Shader Objects");
	uint shaderObjectsArraySize = json_array_get_count(shaderObjectsArray);
	for (uint i = 0; i < shaderObjectsArraySize; i++)
		files.push_back(json_array_get_string(shaderObjectsArray, i));

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
		char* buf = new char[size + 1];
		memcpy(buf, buffer, size);
		buf[size] = 0;

		shaderObject->SetSource(buf, size);
		RELEASE_ARRAY(buf);

		// Try to compile the shader object
		ret = shaderObject->shaderObject = ResourceShaderObject::Compile(shaderObject->GetSource(), shaderObject->shaderType);
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

void ShaderImporter::SetBinaryFormats(GLint formats)
{
	this->formats = formats;
}

GLint ShaderImporter::GetBinaryFormats() const
{
	return formats;
}

void ShaderImporter::LoadDefaultShader()
{
	LoadDefaultVertexShaderObject();
	LoadDefaultFragmentShaderObject();
	LoadDefaultShaderProgram(defaultVertexShaderObject, defaultFragmentShaderObject);
}

void ShaderImporter::LoadCubemapShader()
{
	uint cubemapVShaderObject = ResourceShaderObject::Compile(cubemapvShader, ShaderType::VertexShaderType);
	LoadCubemapShaderProgram(cubemapVShaderObject, defaultFragmentShaderObject);
	glDeleteShader(cubemapVShaderObject);
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

	//defaultShaderProgram = ResourceShaderProgram::Link(shaderObjects); //TODO
}

void ShaderImporter::LoadCubemapShaderProgram(uint vertexShaderObject, uint FragmentShaderObject)
{
	std::list<GLuint> shaderObjects;
	shaderObjects.push_back(vertexShaderObject);
	shaderObjects.push_back(FragmentShaderObject);

	//cubemapShaderProgram = ResourceShaderProgram::Link(shaderObjects); //TODO
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

GLuint ShaderImporter::GetCubemapShaderProgram() const
{
	return cubemapShaderProgram;
}
