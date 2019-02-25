#include "ResourceBone.h"
#include "imgui/imgui.h"
#include "ModuleScene.h"
#include "BoneImporter.h"

ResourceBone::ResourceBone(ResourceTypes type, uint uuid, ResourceData data, ResourceBoneData boneData) : Resource(type, uuid, data), boneData(boneData) {}

ResourceBone::~ResourceBone()
{
}

bool ResourceBone::LoadInMemory()
{
	return true;
}

bool ResourceBone::UnloadFromMemory()
{
	return true;
}

void ResourceBone::OnPanelAssets()
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
		ImGui::SetDragDropPayload("BONE_RESOURCE", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
}


bool ResourceBone::ImportFile(const char * file, std::string & name, std::string & outputFile)
{

	assert(file != nullptr);


	return true;
}

bool ResourceBone::ExportFile(ResourceData & data, ResourceBoneData & bone_data, std::string & outputFile, bool overwrite)
{
	return App->boneImporter->SaveBone(data, bone_data, outputFile, overwrite);
}

uint ResourceBone::CreateMeta(const char * file, uint prefab_uuid, std::string & name, std::string & outputMetaFile)
{
	return uint();
}

bool ResourceBone::ReadMeta(const char * metaFile, int64_t & lastModTime, uint & prefab_uuid, std::string & name)
{
	return false;
}

bool ResourceBone::LoadFile(const char * file, ResourceBoneData & prefab_data_output)
{
	return false;
}

