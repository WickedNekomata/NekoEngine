#include "ComponentBone.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "SceneImporter.h"
#include "GameObject.h"
#include "ResourceMesh.h"
#include "ResourceBone.h"

#include "imgui\imgui.h"
//#include "BoneImporter.h"

ComponentBone::ComponentBone(GameObject * embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::BoneComponent)
{
}

ComponentBone::ComponentBone(GameObject * embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::BoneComponent)
{
	res = resource;
}

ComponentBone::ComponentBone(const ComponentBone& component_bone, GameObject* parent, bool include) : Component(parent, ComponentTypes::BoneComponent)
{
	res = component_bone.res;
	attachedMesh = component_bone.attachedMesh;
}

ComponentBone::~ComponentBone()
{

}

void ComponentBone::OnEditor()
{
	OnUniqueEditor();
}

void ComponentBone::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Bone", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Bone");
		ImGui::SameLine();

		std::string fileName = "Empty Bone";
		const ResourceBone* resource = (ResourceBone*)App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		ImGui::Text("Bone name: %s", resource->boneData.name);
		ImGui::Text("Mesh UUID reference: %i",resource->boneData.mesh_uid);
		ImGui::Text("Bone weights size: %i", resource->boneData.bone_weights_size);

		ImGui::PushID("bone");
		ImGui::Button(fileName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}
	}
#endif
}

uint ComponentBone::GetInternalSerializationBytes()
{
	return sizeof(uint) + sizeof(uint);
}

void ComponentBone::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &attachedMesh, bytes);
	cursor += bytes;
}

void ComponentBone::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResource(loadedRes);

	bytes = sizeof(uint);
	memcpy(&attachedMesh, cursor, bytes);
	cursor += bytes;
}

bool ComponentBone::SetResource(uint resource) //check all this
{
	res = resource;

	return true;
}
