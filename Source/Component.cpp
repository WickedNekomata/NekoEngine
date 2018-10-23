#include "Component.h"
#include "GameObject.h"

Component::Component(GameObject* parent, ComponentType type) : parent(parent), type(type)
{
}

Component::~Component()
{
}

void Component::Update() const
{
}

void Component::OnEditor() const
{


	OnUniqueEditor();
}

void Component::OnUniqueEditor() const {}

ComponentType Component::GetType()
{
	return type;
}

GameObject* Component::GetParent()
{
	return parent;
}

