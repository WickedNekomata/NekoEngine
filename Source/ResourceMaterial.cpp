#include "ResourceMaterial.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceMaterial::ResourceMaterial(ResourceTypes type, uint uuid, ResourceData data, ResourceMaterialData materialData) : Resource(type, uuid, data), materialData(materialData) {}

ResourceMaterial::~ResourceMaterial() {}

void ResourceMaterial::OnPanelAssets()
{
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
		ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
}

// ----------------------------------------------------------------------------------------------------

bool ResourceMaterial::ImportFile(const char* file, std::string& name, std::string& outputFile)
{
	assert(file != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		// Read the meta
		uint uuid = 0;
		int64_t lastModTime = 0;
		ResourceMaterial::ReadMeta(metaFile, lastModTime, uuid, name);
		assert(uuid > 0 && lastModTime > 0);

		// The uuid of the resource would be the entry
		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;
}

bool ResourceMaterial::ExportFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite)
{
	return SaveFile(data, materialData, outputFile, overwrite);
}

bool ResourceMaterial::SaveFile(ResourceData& data, ResourceMaterialData& materialData, std::string& outputFile, bool overwrite)
{
	bool ret = false;

	// Store material data
	uint size = sizeof(materialData);
	char* buffer = new char[size];
	memcpy(buffer, &materialData, size);

	// --------------------------------------------------

	// Build the path of the file
	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	// Save the file
	ret = App->fs->SaveInGame(buffer, size, FileTypes::MaterialFile, outputFile, overwrite) > 0;

	if (ret)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully saved Material '%s'", outputFile.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not save Material '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return ret;
}

bool ResourceMaterial::LoadFile(const char* file, ResourceMaterialData& outputMaterialData)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		// Retrieve material data
		memcpy(&outputMaterialData, buffer, size);

		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully loaded Material '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SHADER IMPORTER: Could not load Material '%s'", file);

	return ret;
}

uint ResourceMaterial::CreateMeta(const char* file, uint materialUuid, std::string& name, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char materialName[DEFAULT_BUF_SIZE];
	strcpy_s(materialName, DEFAULT_BUF_SIZE, name.data());

	// --------------------------------------------------

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(char) * nameSize;

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

	// 3. Store material uuid
	bytes = sizeof(uint) * uuidsSize;
	memcpy(cursor, &materialUuid, bytes);

	cursor += bytes;

	// 4. Store material name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store material name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, materialName, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceMaterial::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& materialUuid, std::string& name)
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
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load material uuid
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&materialUuid, cursor, bytes);

		cursor += bytes;

		// 4. Load material name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load material name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Material: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Material: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

void ResourceMaterial::SetShaderUuid(uint shaderUuid)
{
	materialData.shaderUuid = shaderUuid;
}

uint ResourceMaterial::GetShaderUuid() const
{
	return materialData.shaderUuid;
}

// ----------------------------------------------------------------------------------------------------

void ResourceMaterial::EditTextureMatrix(uint textureUuid)
{

}

bool ResourceMaterial::LoadInMemory()
{
	bool ret = false;

	ret = App->res->SetAsUsed(materialData.shaderUuid);
	if (materialData.albedoUuid > 0)
		App->res->SetAsUsed(materialData.albedoUuid);
	if (materialData.specularUuid > 0)
		App->res->SetAsUsed(materialData.specularUuid);
	if (materialData.normalMapUuid > 0)
		App->res->SetAsUsed(materialData.normalMapUuid);

	return ret;
}

bool ResourceMaterial::UnloadFromMemory()
{
	bool ret = false;

	ret = App->res->SetAsUnused(materialData.shaderUuid);
	if (materialData.albedoUuid > 0)
		App->res->SetAsUnused(materialData.albedoUuid);
	if (materialData.specularUuid > 0)
		App->res->SetAsUnused(materialData.specularUuid);
	if (materialData.normalMapUuid > 0)
		App->res->SetAsUnused(materialData.normalMapUuid);

	return ret;
}