#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "Resource.h"
#include "ModuleScene.h"

#include "imgui\imgui.h"

#include <assert.h>

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent) 
{
	SetResource(App->resHandler->defaultMaterial);
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial, GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent)
{
	if (App->res->GetResource(componentMaterial.res) != nullptr)
		SetResource(componentMaterial.res);
	else
		SetResource(App->resHandler->defaultMaterial);
}

ComponentMaterial::~ComponentMaterial() 
{
	SetResource(0);
}

void ComponentMaterial::Update() {}

void ComponentMaterial::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const Resource* resource = App->res->GetResource(res);
		if (!resource)
			return;
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

		ImGui::SameLine();

		if (res != App->resHandler->defaultMaterial)
		{
			if (ImGui::Button("EDIT"))
				SELECT(App->res->GetResource(res));
		}

		if (ImGui::SmallButton("Use default material"))
			SetResource(App->resHandler->defaultMaterial);
	}
#endif
}

uint ComponentMaterial::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentMaterial::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;
}

void ComponentMaterial::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(uint);
	uint resource = 0;
	memcpy(&resource, cursor, bytes);

	if (App->res->GetResource(resource) != nullptr)
		SetResource(resource);
	else
		SetResource(App->resHandler->defaultMaterial);

	cursor += bytes;
}

// ----------------------------------------------------------------------------------------------------

void ComponentMaterial::SetResource(uint materialUuid)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	res = materialUuid;
}