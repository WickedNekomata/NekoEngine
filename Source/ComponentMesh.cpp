#include "ComponentMesh.h"
#include "GameObject.h"
#include "SceneImporter.h"
#include "Primitive.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleResourceManager.h"

#include "imgui/imgui.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(parent, ComponentType::Mesh_Component)
{
}

ComponentMesh::ComponentMesh(const ComponentMesh& componentMesh) : Component(componentMesh.parent, ComponentType::Mesh_Component)
{
	SetResource(componentMesh.res);
}

ComponentMesh::~ComponentMesh()
{
	parent->meshRenderer = nullptr;
	parent->RecursiveRecalculateBoundingBoxes();

	SetResource(0);
}

void ComponentMesh::Update() {}

void ComponentMesh::SetResource(uint res_uuid)
{
	if (res != 0)
		App->res->SetAsUnused(res);

	if (res_uuid != 0)
		App->res->SetAsUsed(res_uuid);

	res = res_uuid;
}

void ComponentMesh::OnUniqueEditor()
{
	ImGui::Text("Mesh:");

	ImGui::SameLine();

	ImGui::ColorButton("##currentMesh", ImVec4(0.0f,0.0f,0.0f,0.213f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreview, ImVec2(16, 16));
	
	if (ImGui::IsItemHovered())
	{ 
		ImGui::BeginTooltip();
		ImGui::Text("%u", res);
		ImGui::EndTooltip();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
		{
			
		}
		ImGui::EndDragDropTarget();
	}
}

void ComponentMesh::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "ResourceMesh", res);
}

void ComponentMesh::OnLoad(JSON_Object* file)
{
	SetResource(json_object_get_number(file, "ResourceMesh"));
}