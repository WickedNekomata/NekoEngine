#include "ComponentMesh.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMesh::ComponentMesh(GameObject* parent, ComponentType type) : Component(parent, type)
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
