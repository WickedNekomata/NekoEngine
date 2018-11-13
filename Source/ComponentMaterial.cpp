#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "MaterialImporter.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "Resource.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::Material_Component) 
{
	res.reserve(App->renderer3D->GetMaxTextureUnits());
	res.push_back(0);
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentType::Material_Component)
{
	res = componentMaterial.res;
}

ComponentMaterial::~ComponentMaterial()
{
	for (int i = 0; i < res.size(); ++i)
		SetResource(0, i);
	parent->materialRenderer = nullptr;
}

void ComponentMaterial::Update() {}

void ComponentMaterial::SetResource(uint res_uuid, uint position)
{
	if (res[position] != 0)
		App->res->SetAsUnused(res[position]);

	if (res_uuid != 0)
		App->res->SetAsUsed(res_uuid);

	res[position] = res_uuid;
}

void ComponentMaterial::OnUniqueEditor()
{
	ImGui::Text("Material");
	ImGui::Spacing();

	for (uint i = 0; i < res.size(); ++i)
	{
		ImGui::Text("Texture %i", i + 1);
		ImGui::SameLine();

		std::string fileName;
		const Resource* resource = App->res->GetResource(res[i]);
		if (resource != nullptr)
			App->fs->GetFileName(resource->GetFile(), fileName);

		char itemName[DEFAULT_BUF_SIZE];
		sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##%i", fileName.data(), i);
		ImGui::Button(itemName, ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", (res.size() > 0) ? res.front() : 0);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetResource(payload_n, i);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "-##%i", i);
		if (ImGui::Button(itemName))
		{
			SetResource(0, i);
			res.erase(std::remove(res.begin(), res.end(), res[i]));
		}
	}

	if (res.size() < App->renderer3D->GetMaxTextureUnits())
	{
		if (ImGui::Button("+"))
			res.push_back(0);
	}
}

void ComponentMaterial::OnInternalSave(JSON_Object* file)
{
	// TODO
	json_object_set_number(file, "ResourceMaterial", 0012013);
}

void ComponentMaterial::OnLoad(JSON_Object* file)
{
	// TODO LOAD MATERIAL
}