#include "ComponentMaterial.h"

#include "Application.h"
#include "MaterialImporter.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "Resource.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::Material_Component) {}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentType::Material_Component)
{
	res = componentMaterial.res;
}

ComponentMaterial::~ComponentMaterial()
{
	parent->materialRenderer = nullptr;
}

void ComponentMaterial::Update() {}

void ComponentMaterial::SetResource(uint res_uuid)
{
	
}

void ComponentMaterial::OnUniqueEditor()
{
	bool isActive = IsActive();
	if (ImGui::Checkbox("##isActive", &isActive)) { ToggleIsActive(); }
	ImGui::SameLine();

	ImGui::Text("Material");
	ImGui::Separator();
	ImGui::Spacing();

	//if (res.size() < App->renderer3D->get)
	ImGui::Text("Texture");
	ImGui::SameLine();
	const char* name = nullptr;
	ImGui::Button("##name", ImVec2(150.0f, 0.0f));

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
			
		}
		ImGui::EndDragDropTarget();
	}
}

void ComponentMaterial::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "ResourceMaterial", 0012013);
}

void ComponentMaterial::OnLoad(JSON_Object* file)
{
	//LOAD MATERIAL
}