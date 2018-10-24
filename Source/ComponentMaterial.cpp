#include "ComponentMaterial.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::Material_Component)
{
}

ComponentMaterial::~ComponentMaterial()
{
}

void ComponentMaterial::Update() const
{
}

void ComponentMaterial::OnUniqueEditor()
{
	ImGui::Text("THIS IS A MATERIAL COMPONENT");
}
