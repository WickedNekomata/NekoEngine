#include "ComponentMaterial.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent, ComponentType type) : Component(parent, type)
{
}

ComponentMaterial::~ComponentMaterial()
{
}

void ComponentMaterial::Update() const
{
}

void ComponentMaterial::OnEditor() const
{
	ImGui::Text("THIS IS A MATERIAL COMPONENT");
}
