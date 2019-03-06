#include "ResourceScene.h"
#include "imgui/imgui.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include <assert.h>

ResourceScene::ResourceScene(uint uuid, ResourceData data, SceneData customData) : Resource(ResourceTypes::SceneResource, uuid, data)
{
	sceneData = customData;
}

ResourceScene::~ResourceScene()
{

}

void ResourceScene::OnPanelAssets()
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

	if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
	{
		System_Event event;
		event.sceneEvent.type = System_Event_Type::LoadScene;

		std::string fileName;
		App->fs->GetFileName(data.name.data(), fileName);

		memcpy(event.sceneEvent.nameScene, fileName.data(), sizeof(char) * DEFAULT_BUF_SIZE);
		App->PushSystemEvent(event);
	}

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		Resource* res = this;
		ImGui::SetDragDropPayload("SCENE_RESOURCE", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();
	}
#endif
}

bool ResourceScene::GenerateLibraryFiles() const
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
			sprintf_s(newMetaFile, "%s/%s%s", DIR_LIBRARY_SCENES, data.name.data(), EXTENSION_META);

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	//2 Copy scene file
	// Read the info of the meta
	if (App->fs->Exists(data.file.data()))
	{
		char* buffer;
		uint size = App->fs->Load(data.file.data(), &buffer);
		if (size > 0)
		{
			char newFile[DEFAULT_BUF_SIZE];
			sprintf_s(newFile, "%s/%s", DIR_LIBRARY_SCENES, data.name.data());

			// Save the new file
			size = App->fs->Save(newFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}
	return true;
}

ResourceScene* ResourceScene::ImportFile(const char* file)
{
	std::string ext;
	App->fs->GetExtension(file, ext);

	assert(App->res->GetResourceTypeByExtension(ext.data()) == ResourceTypes::SceneResource);

	std::string fileName;
	App->fs->GetFileName(file, fileName, true);

	ResourceData data;
	data.file = file;
	data.exportedFile = "";
	data.name = fileName;

	ResourceScene* retScene = new ResourceScene(App->GenerateRandomNumber(), data, SceneData());

	retScene->UpdateFromMeta();

	return retScene;
}

ResourceScene* ResourceScene::ExportFile(const char* sceneName)
{
	ResourceData data;
	data.file = DIR_ASSETS_SCENES + std::string("/") + sceneName + EXTENSION_SCENE;
	data.exportedFile = "";
	data.name = sceneName + std::string(EXTENSION_SCENE);

	ResourceScene* retScene = (ResourceScene*)App->res->CreateResource(ResourceTypes::SceneResource, data, &SceneData());

	char* buffer;
	uint size;
	App->GOs->SerializeFromNode(App->scene->root, buffer, size, true);

	App->fs->Save(data.file, buffer, size);

	//Create the .meta
	int64_t lastModTime = App->fs->GetLastModificationTime(data.file.data());
	CreateMeta(retScene, lastModTime);

	return retScene;
}

bool ResourceScene::CreateMeta(ResourceScene* scene, int64_t lastModTime)
{
	uint size = ResourceScene::GetMetaSize();
	char* metaBuffer = new char[size];
	char* cursor = metaBuffer;

	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);
	cursor += bytes;

	bytes = sizeof(uint);

	uint temp = 0;
	memcpy(cursor, &temp, bytes);
	cursor += bytes;

	temp = scene->GetUuid();
	memcpy(cursor, &temp, bytes);
	cursor += bytes;

	App->fs->Save(scene->data.file + EXTENSION_META, metaBuffer, size);

	return true;
}

bool ResourceScene::UpdateFromMeta()
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

bool ResourceScene::LoadInMemory()
{
	return true;
}

bool ResourceScene::UnloadFromMemory()
{
	return true;
}

uint ResourceScene::GetSceneBuffer(char*& buffer) const
{
	return App->fs->Load(data.file, &buffer);
}