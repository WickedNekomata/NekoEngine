#include "ComponentButton.h"

#include "GameObject.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentButton::ComponentButton(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::ButtonComponent)
{
}

ComponentButton::ComponentButton(const ComponentButton & componentRectTransform) : Component(parent, ComponentTypes::ButtonComponent)
{

}

ComponentButton::~ComponentButton()
{
}

void ComponentButton::Update()
{
}

uint ComponentButton::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentButton::OnInternalSave(char *& cursor)
{
}

void ComponentButton::OnInternalLoad(char *& cursor)
{
}

void ComponentButton::OnUniqueEditor()
{
}

