#include "ResourcePrefab.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include <assert.h>

ResourcePrefab::ResourcePrefab(uint uuid, ResourceData data, PrefabData customData) : Resource(ResourceTypes::PrefabResource, uuid, data)
{
	prefabData = customData;
}

ResourcePrefab::~ResourcePrefab()
{
	if (prefabData.root != nullptr)
	{
		prefabData.root->DestroyTemplate();
	}
}

void ResourcePrefab::OnPanelAssets()
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
		Resource* res = this;
		ImGui::SetDragDropPayload("PREFAB_RESOURCE", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();
	}
#endif
}

bool ResourcePrefab::GenerateLibraryFiles() const
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

			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%s%s", DIR_LIBRARY_PREFAB, data.name.data(), EXTENSION_META);

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	//2 Copy prefab file
	// Read the info of the meta
	if (App->fs->Exists(data.file.data()))
	{
		char* buffer;
		uint size = App->fs->Load(data.file.data(), &buffer);
		if (size > 0)
		{
			char newFile[DEFAULT_BUF_SIZE];
			sprintf_s(newFile, "%s/%s", DIR_LIBRARY_PREFAB, data.name.data());

			// Save the new file
			size = App->fs->Save(newFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	return true;
}

ResourcePrefab* ResourcePrefab::ImportFile(const char* file)
{
	std::string ext;
	App->fs->GetExtension(file, ext);

	assert(App->res->GetResourceTypeByExtension(ext.data()) == ResourceTypes::PrefabResource);

	std::string fileName;
	App->fs->GetFileName(file, fileName, true);

	ResourceData data;
	data.file = file;
	data.exportedFile = "";
	data.name = fileName;

	ResourcePrefab* retPrefab = new ResourcePrefab(App->GenerateRandomNumber(), data, PrefabData());

	retPrefab->UpdateFromMeta();

	return retPrefab;
}

ResourcePrefab* ResourcePrefab::ExportFile(const char* prefabName, GameObject* templateRoot)
{
	char filePath[DEFAULT_BUF_SIZE];
	sprintf(filePath, "%s/%s%s", DIR_ASSETS_PREFAB, prefabName, EXTENSION_PREFAB);

	ResourceData data;
	data.file = DIR_ASSETS_PREFAB + std::string("/") + prefabName + EXTENSION_PREFAB;
	data.exportedFile = "";
	data.name = prefabName + std::string(EXTENSION_PREFAB);

	ResourcePrefab* retPrefab = new ResourcePrefab(App->GenerateRandomNumber(), data, PrefabData());

	char* buffer;
	uint size;
	App->GOs->SerializeFromNode(templateRoot, buffer, size);

	App->fs->Save(data.file, buffer, size);

	//Create the .meta
	int64_t lastModTime = App->fs->GetLastModificationTime(data.file.data());
	CreateMeta(retPrefab, lastModTime);

	return retPrefab;
}

bool ResourcePrefab::CreateMeta(ResourcePrefab* prefab, int64_t lastModTime)
{
	uint size = ResourcePrefab::GetMetaSize();
	char* metaBuffer = new char[size];
	char* cursor = metaBuffer;

	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);
	cursor += bytes;

	bytes = sizeof(uint);

	uint temp = 0;
	memcpy(cursor, &temp, bytes);
	cursor += bytes;

	temp = prefab->GetUuid();
	memcpy(cursor, &temp, bytes);
	cursor += bytes;

	App->fs->Save(prefab->data.file + EXTENSION_META, metaBuffer, size);

	return true;
}

bool ResourcePrefab::UpdateFromMeta()
{
	if (App->fs->Exists(data.file + EXTENSION_META))
	{
		char* buffer;
		uint size = App->fs->Load(data.file + EXTENSION_META, &buffer);
		assert(size > 0);

		char* cursor = buffer;
		uint bytes = sizeof(uint);
		cursor += sizeof(int64_t) + bytes;

		memcpy(&uuid, cursor, bytes);
		cursor += bytes;

		delete[] buffer;
	}
	else
	{
		CreateMeta(this, App->fs->GetLastModificationTime(data.file.data()));
	}

	return true;
}

bool ResourcePrefab::UpdateRoot()
{
	if (prefabData.root)
	{
		UnloadFromMemory();
		LoadInMemory();
	}

	return true;
}

bool ResourcePrefab::LoadInMemory()
{
	assert(!prefabData.root);

	char* buffer;
	uint size = App->fs->Load(data.file, &buffer);
	if (size <= 0)
		return false;

	GameObject* temp = App->GOs->DeSerializeToNode(buffer, size);

	std::vector<GameObject*> childs;
	temp->GetChildrenVector(childs, true);

	System_Event event;
	event.type = System_Event_Type::LoadFinished;

	for(auto gameObject : childs)
		gameObject->OnSystemEvent(event);

	prefabData.root = new GameObject(*temp, false);
	prefabData.root->ForceUUID(uuid);
	prefabData.root->prefab = this;

	delete[] buffer;
	temp->DestroyTemplate();

	return true;
}

bool ResourcePrefab::UnloadFromMemory()
{
	if (prefabData.root)
	{
		prefabData.root->DestroyTemplate();
		prefabData.root = nullptr;
	}

	return true;
}
