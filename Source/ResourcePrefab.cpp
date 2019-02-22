#include "ResourcePrefab.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleFileSystem.h"

ResourcePrefab::ResourcePrefab(uint uuid, ResourceData data, PrefabData customData) : Resource(ResourceTypes::PrefabResource, uuid, data)
{
}

ResourcePrefab::~ResourcePrefab()
{
	//TODO: DESTROY THE ROOT IF LOADED
}

void ResourcePrefab::OnPanelAssets()
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
		Resource* res = this;
		ImGui::SetDragDropPayload("PREFAB_RESOURCE", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();

		//TODO: RECEIVE THIS DRAG AND DROP IN THE HIERARCHY AND INSTANTIATE A COPY OF THE LOADED ROOT WITH THE COPY CONSTRUCTOR
	}
}

bool ResourcePrefab::ExportFile(ResourceData & data, PrefabData & prefabData, std::string & outputFile, bool overwrite)
{
	bool ret = false;
	char* buffer;
	uint size = 0u;
	App->GOs->SerializeFromNode(prefabData.root, buffer, size);

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	ret = App->fs->SaveInGame(buffer, size, FileType::PrefabFile, outputFile, overwrite) > 0;

	if (ret) {
		CONSOLE_LOG(LogTypes::Normal, "PREFAB EXPORT: Successfully saved prefab '%s'", outputFile.data());
	}else
		CONSOLE_LOG(LogTypes::Normal, "PREFAB EXPORT: Error saving prefab '%s'", outputFile.data());

	RELEASE_ARRAY(buffer);

	return true;
}

bool ResourcePrefab::LoadInMemory()
{
	//TODO: LOAD THE ROOT
	return true;
}

bool ResourcePrefab::UnloadFromMemory()
{
	//TODO: DELETE THE ROOT FROM MEMORY
	return true;
}
