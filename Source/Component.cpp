#include "Component.h"
#include "GameObject.h"

Component::Component(GameObject* parent, ComponentType type) : parent(parent), type(type)
{
}

Component::~Component()
{
}

void Component::Update()
{

}

void Component::OnEditor()
{


	OnUniqueEditor();
}

void Component::OnUniqueEditor() {}

ComponentType Component::GetType()
{
	return type;
}

GameObject* Component::GetParent()
{
	return parent;
}

void Component::OnSave(JSON_Object* file)
{
}

