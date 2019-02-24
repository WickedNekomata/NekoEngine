#include "ResourceAnimation.h"
#include "imgui/imgui.h"
#include "ModuleScene.h"

ResourceAnimation::ResourceAnimation(ResourceTypes type, uint uuid, ResourceData data, ResourceAnimationData animationData) : Resource(type, uuid, data), animationData(animationData) {}

ResourceAnimation::~ResourceAnimation()
{
}

bool ResourceAnimation::LoadInMemory()
{
	return true;
}

bool ResourceAnimation::UnloadFromMemory()
{
	return true;
}

void ResourceAnimation::OnPanelAssets()
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
		ImGui::SetDragDropPayload("ANIMATION_RESOURCE", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
}

bool ResourceAnimation::ImportFile(const char* file, std::string& name, std::string& outputFile)
{
	return false;
}

bool ResourceAnimation::ExportFile(ResourceData& data, ResourceAnimationData& prefabData, std::string& outputFile, bool overwrite)
{
	return false;
}

uint ResourceAnimation::CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile)
{
	return uint();
}

bool ResourceAnimation::ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name)
{
	return false;
}

bool ResourceAnimation::LoadFile(const char* file, ResourceAnimationData& prefab_data_output)
{
	return false;
}
