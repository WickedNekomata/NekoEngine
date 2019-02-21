#include "GameMode.h"

#include "ShaderImporter.h"

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"
// TODO ERASE THIS
#include "ModuleInternalResHandler.h"

#include <assert.h>

ShaderImporter::ShaderImporter() {}

ShaderImporter::~ShaderImporter()
{
	ResourceShaderProgram::DeleteShaderProgram(defaultShaderProgram);
	ResourceShaderProgram::DeleteShaderProgram(cubemapShaderProgram);

	ResourceShaderObject::DeleteShaderObject(defaultVertexShaderObject);
	ResourceShaderObject::DeleteShaderObject(defaultFragmentShaderObject);
}

bool ShaderImporter::SaveShaderObject(ResourceData& data, ResourceShaderObjectData& outputShaderObjectData, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	uint size = strlen(outputShaderObjectData.GetSource());
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully read Shader Object '%s'", outputFile.data());
		ret = SaveShaderObject(outputShaderObjectData.GetSource(), size, outputShaderObjectData.shaderType, outputFile, overwrite);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not read Shader Object '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderObject(const void* buffer, uint size, ShaderTypes shaderType, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	FileType fileType = FileType::NoFileType;
	switch (shaderType)
	{
	case ShaderTypes::VertexShaderType:
		fileType = FileType::VertexShaderObjectFile;
		break;
	case ShaderTypes::FragmentShaderType:
		fileType = FileType::FragmentShaderObjectFile;
		break;
	}

	if (App->fs->SaveInGame((char*)buffer, size, fileType, outputFile, overwrite) > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully saved Shader Object '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not save Shader Object '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(ResourceData& data, ResourceShaderProgramData& outputShaderProgramData, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	// Verify that the driver supports at least one shader binary format
	if (GetBinaryFormats() == 0)
		return ret;

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	uchar* buffer;
	uint size = ResourceShaderProgram::GetBinary(ResourceShaderProgram::Link((outputShaderProgramData.shaderObjects)), &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully got Binary Program '%s'", outputFile.data());
		ret = SaveShaderProgram(buffer, size, outputFile, overwrite);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not get Binary Program '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::SaveShaderProgram(const void* buffer, uint size, std::string& outputFile, bool overwrite) const
{
	bool ret = false;

	// Verify that the driver supports at least one shader binary format
	if (GetBinaryFormats() == 0)
		return ret;

	if (App->fs->SaveInGame((char*)buffer, size, FileType::ShaderProgramFile, outputFile, overwrite) > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully saved Binary Program '%s'", outputFile.data());
		ret = true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not save Binary Program '%s'", outputFile.data());

	return ret;
}

bool ShaderImporter::LoadShaderObject(const char* objectFile, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const
{
	bool ret = false;

	assert(objectFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(objectFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully loaded Shader Object '%s'", objectFile);
		ret = LoadShaderObject(buffer, size, outputShaderObjectData, shaderObject);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not load Shader Object '%s'", objectFile);

	return ret;
}

bool ShaderImporter::LoadShaderObject(const void* buffer, uint size, ResourceShaderObjectData& outputShaderObjectData, uint& shaderObject) const
{
	assert(buffer != nullptr && size > 0);

	char* buf = new char[size + 1];
	memcpy(buf, buffer, size);
	buf[size] = 0;

	outputShaderObjectData.SetSource(buf, size);
	RELEASE_ARRAY(buf);

	// Try to compile the shader object
	shaderObject = ResourceShaderObject::Compile(outputShaderObjectData.GetSource(), outputShaderObjectData.shaderType);

	if (shaderObject > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: New Shader Object loaded with: size %u", size);
		return true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Shader Object with size %u could not be loaded", size);

	return false;
}

bool ShaderImporter::LoadShaderProgram(const char* programFile, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const
{
	bool ret = false;

	assert(programFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(programFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: Successfully loaded Shader Program '%s'", programFile);
		ret = LoadShaderProgram(buffer, size, outputShaderProgramData, shaderProgram);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not load Shader Program '%s'", programFile);

	return ret;
}

bool ShaderImporter::LoadShaderProgram(const void* buffer, uint size, ResourceShaderProgramData& outputShaderProgramData, uint& shaderProgram) const
{
	assert(buffer != nullptr && size > 0);

	// Try to link the shader program
	shaderProgram = ResourceShaderProgram::LoadBinary(buffer, size);

	if (shaderProgram > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SHADER IMPORTER: New Shader Program loaded with: size %u", size);
		return true;
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Shader Program with size %u could not be loaded", size);

	return false;
}

// ----------------------------------------------------------------------------------------------------

void ShaderImporter::SetBinaryFormats(int formats)
{
	this->formats = formats;
}

int ShaderImporter::GetBinaryFormats() const
{
	return formats;
}

// ----------------------------------------------------------------------------------------------------

void ShaderImporter::LoadDefaultShader()
{
	defaultVertexShaderObject = LoadDefaultShaderObject(ShaderTypes::VertexShaderType);
	defaultFragmentShaderObject = LoadDefaultShaderObject(ShaderTypes::FragmentShaderType);
	defaultShaderProgram = LoadShaderProgram(defaultVertexShaderObject, defaultFragmentShaderObject);
}

void ShaderImporter::LoadCubemapShader()
{
	uint cubemapVertexShaderObject = ResourceShaderObject::Compile(cubemapvShader, ShaderTypes::VertexShaderType);
	uint cubemapFragmentShaderObject = ResourceShaderObject::Compile(cubemapfShader, ShaderTypes::FragmentShaderType);
	cubemapShaderProgram = LoadShaderProgram(cubemapVertexShaderObject, cubemapFragmentShaderObject);
	ResourceShaderObject::DeleteShaderObject(cubemapVertexShaderObject);
	ResourceShaderObject::DeleteShaderObject(cubemapFragmentShaderObject);
}

uint ShaderImporter::LoadDefaultShaderObject(ShaderTypes shaderType) const
{
	const char* source = nullptr;

	switch (shaderType)
	{
	case ShaderTypes::VertexShaderType:
		source = vShaderTemplate;
		break;
	case ShaderTypes::FragmentShaderType:
		source = fShaderTemplate;
		break;
	}

	return ResourceShaderObject::Compile(source, shaderType);
}

#include "glew/include/GL/glew.h"

uint ShaderImporter::LoadShaderProgram(uint vertexShaderObject, uint fragmentShaderObject) const
{
	// Create a Shader Program
	GLuint shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShaderObject);
	glAttachShader(shaderProgram, fragmentShaderObject);

	// Link the Shader Program
	glLinkProgram(shaderProgram);

	glAttachShader(shaderProgram, vertexShaderObject);
	glDetachShader(shaderProgram, fragmentShaderObject);

	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* infoLog = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);

		CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "Successfully linked Shader Program");

	return shaderProgram;
}

uint ShaderImporter::GetDefaultVertexShaderObject() const
{
	return defaultVertexShaderObject;
}

uint ShaderImporter::GetDefaultFragmentShaderObject() const
{
	return defaultFragmentShaderObject;
}

uint ShaderImporter::GetDefaultShaderProgram() const
{
	return defaultShaderProgram;
}

uint ShaderImporter::GetCubemapShaderProgram() const
{
	return cubemapShaderProgram;
}