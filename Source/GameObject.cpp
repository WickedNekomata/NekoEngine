#include "GameObject.h"

#include "Component.h"

GameObject::GameObject(char* name, GameObject* parent) : name(name), parent(parent)
{
}

GameObject::~GameObject()
{
	DeleteComponents();
	DeleteChildren();
}

void GameObject::Update() const
{
}

void inline GameObject::SetParent(GameObject* parent)
{
	// TODO: Reset childrens' transform
	this->parent = parent;
}

void GameObject::AddChild(GameObject* children)
{
	this->children.push_back(children);
}

void GameObject::EraseChild(uint index)
{
	components.erase(components.begin() + index);
}

void GameObject::DeleteChild(uint index)
{
	delete components[index];
	components.erase(components.begin() + index);
}

void GameObject::DeleteChildren()
{
	for (int i = 0; i < children.size(); ++i)
		delete children[i];

	children.clear();
}

bool GameObject::HasChildren() const
{
	return children.size() > 0;
}

uint GameObject::GetChildrenLength() const
{
	return children.size();
}

GameObject* GameObject::GetChild(uint index) const
{
	return children[index];
}

void GameObject::AddComponent(ComponentType type)
{
	// TODO: Add type as argument to Component constructor
	Component* newComponent = new Component(this);
	
	components.push_back(newComponent);
}

void GameObject::DeleteComponent(uint index)
{
	delete components[index];
	components.erase(components.begin() + index);
}

void GameObject::DeleteComponents()
{
	for (int i = 0; i < components.size(); ++i)
		delete components[i];

	components.clear();
}

bool GameObject::HasComponents() const
{
	return components.size() > 0;
}

uint GameObject::GetComponenetsLength() const
{
	return components.size();
}

Component* GameObject::GetComponent(uint index) const
{
	return components[index];
}

const char* GameObject::GetName() const
{
	return name;
}

void GameObject::SetName(char* name)
{
	this->name = name;
}


