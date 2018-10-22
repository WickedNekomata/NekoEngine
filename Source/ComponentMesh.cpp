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

void ComponentMesh::OnEditor() const
{
	ImGui::Text("THIS IS A MESH COMPONENT");
}
