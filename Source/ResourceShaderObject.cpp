#include "ResourceShaderObject.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ShaderImporter.h"
#include "ModuleScene.h"

#include "ModuleGui.h"
#include "PanelCodeEditor.h"

#include "glew\include\GL\glew.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceShaderObject::ResourceShaderObject(ResourceTypes type, uint uuid, ResourceData data, ResourceShaderObjectData shaderObjectData) : Resource(type, uuid, data), shaderObjectData(shaderObjectData) {}

ResourceShaderObject::~ResourceShaderObject()
{
	DeleteShaderObject(shaderObject);
}

void ResourceShaderObject::OnPanelAssets()
{
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	ImGui::TreeNodeEx(id, flags);

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		Resource* res = this;
		ImGui::SetDragDropPayload("SHADER_OBJECT", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();
	}
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderObject::ImportFile(const char* file, std::string& outputFile)
{
	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		uint uuid = 0;
		assert(ResourceShaderObject::ReadShaderObjectUuidFromMeta(metaFile, uuid));

		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%s", uuid);

		outputFile = entry;
	}

	return true;
}

bool ResourceShaderObject::ExportFile(ResourceShaderObjectData& shaderObjectData, ResourceData& data, std::string& outputFile, bool overwrite)
{
	return App->shaderImporter->SaveShaderObject(data, shaderObjectData, outputFile, overwrite);
}

// Returns the last modification time of the file
uint ResourceShaderObject::CreateMeta(const char* file, uint shaderObjectUuid, std::string& outputMetaFile)
{
	assert(file != nullptr);
	
	uint uuidsSize = 1;

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize;

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store shader object uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &shaderObjectUuid, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Object: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Object: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceShaderObject::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& shaderObjectUuid)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load shader object uuid
		bytes = sizeof(uint);
		memcpy(&shaderObjectUuid, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Object: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Object: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderObject::Compile()
{
	bool ret = true;

	uint shader = 0;
	switch (shaderObjectData.shaderType)
	{
	case ShaderTypes::VertexShaderType:
		shader = GL_VERTEX_SHADER;
		break;
	case ShaderTypes::FragmentShaderType:
		shader = GL_FRAGMENT_SHADER;
		break;
	}

	// Create a Shader Object
	DeleteShaderObject(shaderObject);
	shaderObject = glCreateShader(shader); // Creates an empty Shader Object
	const char* source = shaderObjectData.GetSource();
	glShaderSource(shaderObject, 1, &source, NULL); // Takes an array of strings and stores it into the shader

	// Compile the Shader Object
	glCompileShader(shaderObject);

	if (!IsObjectCompiled())
	{
		DeleteShaderObject(shaderObject);
		ret = false;
	}

	return ret;
}

uint ResourceShaderObject::Compile(const char* source, ShaderTypes shaderType)
{
	GLenum shader = 0;
	switch (shaderType)
	{
	case ShaderTypes::VertexShaderType:
		shader = GL_VERTEX_SHADER;
		break;
	case ShaderTypes::FragmentShaderType:
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

		CONSOLE_LOG(LogTypes::Error, "Shader Object could not be compiled. ERROR: %s", infoLog);

#ifndef GAMEMODE
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
#endif

		DeleteShaderObject(shaderObject);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "Successfully compiled Shader Object");

	return shaderObject;
}

bool ResourceShaderObject::DeleteShaderObject(uint shaderObject)
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

// ----------------------------------------------------------------------------------------------------

ShaderTypes ResourceShaderObject::GetShaderType() const
{
	return shaderObjectData.shaderType;
}

void ResourceShaderObject::SetSource(const char* source, uint size)
{
	shaderObjectData.SetSource(source, size);
}

const char* ResourceShaderObject::GetSource() const
{
	return shaderObjectData.GetSource();
}

// ----------------------------------------------------------------------------------------------------

bool ResourceShaderObject::ReadShaderObjectUuidFromMeta(const char* metaFile, uint& shaderObjectUuid)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. (Last modification time)
		uint bytes = sizeof(int64_t);
		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load shader object uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&shaderObjectUuid, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Shader Object: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Shader Object: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceShaderObject::LoadInMemory()
{
	return true;
}

bool ResourceShaderObject::UnloadFromMemory()
{
	return true;
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

		CONSOLE_LOG(LogTypes::Normal, "Shader Object could not be compiled. ERROR: %s", infoLog);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Successfully compiled Shader Object");

	return success;
}