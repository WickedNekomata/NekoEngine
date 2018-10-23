#include "ComponentTransform.h"
#include "GameObject.h"

#include "imgui/imgui.h"

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentType::Transform_Component)
{
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::Update() const
{
}

void ComponentTransform::OnUniqueEditor() const
{
	ImGui::Text("This is a Transform Component");
}
