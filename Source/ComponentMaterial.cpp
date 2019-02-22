#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "Resource.h"

#include "imgui\imgui.h"

#include <assert.h>

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent) 
{
	SetResource(App->resHandler->defaultMaterial);
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentTypes::MaterialComponent) {}

ComponentMaterial::~ComponentMaterial() 
{
	SetResource(0);
}

void ComponentMaterial::Update() {}

void ComponentMaterial::SetResource(uint materialUuid)
{
	if (res > 0)
		assert(App->res->SetAsUnused(res) > 0);

	if (materialUuid > 0)
		assert(App->res->SetAsUsed(materialUuid) > 0);

	res = materialUuid;
}

void ComponentMaterial::OnUniqueEditor()
{
#ifndef GAME
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const Resource* resource = App->res->GetResource(res);
		std::string materialName = resource->GetName();

		ImGui::PushID("material");
		ImGui::Button(materialName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetResource(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}

uint ComponentMaterial::GetInternalSerializationBytes()
{
	return 0;
}

void ComponentMaterial::OnInternalSave(char*& cursor)
{
}

void ComponentMaterial::OnInternalLoad(char*& cursor)
{
}