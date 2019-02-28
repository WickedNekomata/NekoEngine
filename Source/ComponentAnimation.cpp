#include "ComponentAnimation.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "SceneImporter.h"
#include "GameObject.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceAnimation.h"
#include "AnimationImporter.h"
#include "ModuleAnimation.h"

#include "imgui\imgui.h"

ComponentAnimation::ComponentAnimation(GameObject * embedded_game_object) :
	Component(embedded_game_object, ComponentTypes::AnimationComponent)
{
}

ComponentAnimation::ComponentAnimation(GameObject* embedded_game_object, uint resource) :
	Component(embedded_game_object, ComponentTypes::AnimationComponent)
{
	res = resource;
}

ComponentAnimation::ComponentAnimation(const ComponentAnimation & component_anim, GameObject * parent, bool include) : Component(parent, ComponentTypes::AnimationComponent)
{
	this->SetResource(component_anim.res);
}

ComponentAnimation::~ComponentAnimation()
{
	
}

uint ComponentAnimation::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentAnimation::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;
}

void ComponentAnimation::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResource(loadedRes);
}

bool ComponentAnimation::SetResource(uint resource) //check all this
{
	res = resource;

	return true;
}

bool ComponentAnimation::PlayAnimation(char * anim_name)
{
	// mimimi
	return true;
}

void ComponentAnimation::OnEditor()
{
	OnUniqueEditor();
}

void ComponentAnimation::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Animation");
		ImGui::SameLine();

		std::string fileName = "Empty Animation";
		const ResourceAnimation* resource = (ResourceAnimation*)App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		ImGui::Text("Animation name: %s", resource->animationData.name);
		ImGui::Text("Animation numKeys: %i", resource->animationData.numKeys);
		ImGui::Text("Animation UUID: %i", resource->GetUuid());

		ImGui::PushID("animation");
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