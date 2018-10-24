#include "ComponentMesh.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(parent, ComponentType::Mesh_Component)
{
}

ComponentMesh::~ComponentMesh()
{
}

void ComponentMesh::Update() const
{
}

void ComponentMesh::OnUniqueEditor()
{
	ImGui::Text("Mesh"); ImGui::SameLine();

	// https://github.com/ocornut/imgui/issues/1566
}
