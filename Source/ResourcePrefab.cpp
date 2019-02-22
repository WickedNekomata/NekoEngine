#include "ResourcePrefab.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"

ResourcePrefab::ResourcePrefab(uint uuid, ResourceData data, PrefabData customData) : Resource(ResourceTypes::PrefabResource, uuid, data)
{


}

ResourcePrefab::~ResourcePrefab()
{
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
