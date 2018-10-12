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

void GameObject::Addchildren(GameObject* children)
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

inline const char* GameObject::GetName() const
{
	return name;
}

inline void GameObject::SetName(char* name)
{
	this->name = name;
}


