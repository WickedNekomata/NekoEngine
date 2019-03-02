#include "ResourceTexture.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "MaterialImporter.h"
#include "ModuleScene.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceTexture::ResourceTexture(ResourceTypes type, uint uuid, ResourceData data, ResourceTextureData textureData) : Resource(type, uuid, data), textureData(textureData) {}

ResourceTexture::~ResourceTexture() {}

void ResourceTexture::OnPanelAssets()
{
#ifndef GAMEMODE
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("TEXTURE_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

bool ResourceTexture::ImportFile(const char* file, ResourceTextureImportSettings& textureImportSettings, std::string& outputFile)
{
	assert(file != nullptr);

	bool imported = false;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// CASE 1 (file). The file has no meta associated (the file is new)
	if (!App->fs->Exists(metaFile))
	{
		// Import the file (using the default import settings)
		CONSOLE_LOG(LogTypes::Normal, "Resource Texture: The file '%s' needs to be imported", file);

		imported = App->materialImporter->Import(file, outputFile, textureImportSettings);
	}
	else
	{
		uint uuid = 0;
		int64_t lastModTime = 0;
		bool result = ResourceTexture::ReadMeta(metaFile, lastModTime, textureImportSettings, uuid);
		assert(result);

		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u%s", uuid, EXTENSION_TEXTURE);

		// CASE 2 (file + meta + Library file(s)). The resource(s) do(es)n't exist
		std::string entryFile = DIR_LIBRARY;
		if (App->fs->RecursiveExists(entry, DIR_LIBRARY, entryFile))
		{
			outputFile = entryFile;
			imported = true;
		}
		// CASE 3 (file + meta). The file(s) in Libray associated to the meta do(es)n't exist
		else
		{
			// Import the file (using the import settings from the meta)
			CONSOLE_LOG(LogTypes::Normal, "Resource Texture: The file '%s' has Library file(s) that need(s) to be reimported", file);

			imported = App->materialImporter->Import(file, outputFile, textureImportSettings, uuid);
		}
	}

	return imported;
}

// Returns the last modification time of the file
uint ResourceTexture::CreateMeta(const char* file, ResourceTextureImportSettings& textureImportSettings, uint textureUuid, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(int) +
		sizeof(int) + 
		sizeof(int) + 
		sizeof(int) + 
		sizeof(int) + 
		sizeof(float);

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
	assert(uuidsSize > 0);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store texture uuid
	bytes = sizeof(uint) * uuidsSize;
	assert(textureUuid > 0);
	memcpy(cursor, &textureUuid, bytes);

	cursor += bytes;

	// 4. Store import settings
	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.compression, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.wrapS, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.wrapT, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.minFilter, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.magFilter, bytes);

	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &textureImportSettings.anisotropy, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Texture: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Texture: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceTexture::ReadMeta(const char* metaFile, int64_t& lastModTime, ResourceTextureImportSettings& textureImportSettings, uint& textureUuid)
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
		assert(lastModTime > 0);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load texture uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&textureUuid, cursor, bytes);
		assert(textureUuid > 0);

		cursor += bytes;

		// 4. Load import settings
		bytes = sizeof(int);
		memcpy(&textureImportSettings.compression, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.wrapS, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.wrapT, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.minFilter, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.magFilter, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(float);
		memcpy(&textureImportSettings.anisotropy, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

uint ResourceTexture::SetTextureImportSettingsToMeta(const char* metaFile, const ResourceTextureImportSettings& textureImportSettings)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	ResourceTextureImportSettings oldTextureImportSettings;
	uint textureUuid = 0;
	ReadMeta(metaFile, lastModTime, oldTextureImportSettings, textureUuid);

	uint uuidsSize = 1;

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(int) +
		sizeof(int) +
		sizeof(int) +
		sizeof(int) +
		sizeof(int) +
		sizeof(float);

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	assert(lastModTime > 0);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	assert(uuidsSize > 0);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store texture uuid
	bytes = sizeof(uint) * uuidsSize;
	assert(textureUuid > 0);
	memcpy(cursor, &textureUuid, bytes);

	cursor += bytes;

	// 4. Store import settings
	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.compression, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.wrapS, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.wrapT, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.minFilter, bytes);

	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &textureImportSettings.magFilter, bytes);

	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &textureImportSettings.anisotropy, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	uint resultSize = App->fs->Save(metaFile, data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Texture: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Texture: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

bool ResourceTexture::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		// Read the info of the meta
		char* buffer;
		uint size = App->fs->Load(metaFile, &buffer);
		if (size > 0)
		{
			// Create a new name for the meta
			std::string extension = metaFile;

			uint found = extension.find_first_of(".");
			if (found != std::string::npos)
				extension = extension.substr(found, extension.size());

			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%u%s", DIR_LIBRARY_TEXTURES, uuid, extension.data());

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				return true;

			RELEASE_ARRAY(buffer);
		}		
	}

	return false;
}

// ----------------------------------------------------------------------------------------------------

uint ResourceTexture::GetId() const
{
	return id;
}

uint ResourceTexture::GetWidth() const
{
	return textureData.width;
}

uint ResourceTexture::GetHeight() const
{
	return textureData.height;
}

// ----------------------------------------------------------------------------------------------------

// Returns true if the value of the texture uuid is > 0. Else, returns false
bool ResourceTexture::ReadTextureUuidFromMeta(const char* metaFile, uint& textureUuid)
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

		// 3. Load texture uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&textureUuid, cursor, bytes);
		assert(textureUuid > 0);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	if (textureUuid > 0)
		return true;

	return false;
}

bool ResourceTexture::ReadTextureImportSettingsFromMeta(const char* metaFile, ResourceTextureImportSettings& textureImportSettings)
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

		// 3. (Texture uuid)
		bytes = sizeof(uint) * uuidsSize;
		cursor += bytes;

		// 4. Load import settings
		bytes = sizeof(int);
		memcpy(&textureImportSettings.compression, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.wrapS, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.wrapT, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.minFilter, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(int);
		memcpy(&textureImportSettings.magFilter, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(float);
		memcpy(&textureImportSettings.anisotropy, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceTexture::LoadInMemory()
{
	return App->materialImporter->Load(data.exportedFile.data(), textureData, id);
}

bool ResourceTexture::UnloadFromMemory()
{
	App->materialImporter->DeleteTexture(id);

	return true;
}