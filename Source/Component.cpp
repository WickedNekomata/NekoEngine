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
}
