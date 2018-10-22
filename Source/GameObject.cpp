#include "GameObject.h"

#include "ComponentMesh.h"
#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include <algorithm>

GameObject::GameObject(char* name, GameObject* parent) : name(name), parent(parent)
{
}

GameObject::~GameObject()
{
	InternallyDeleteComponents();
}

void GameObject::Update() const
{
}

void GameObject::SetParent(GameObject* parent)
{
	// WARNING: Reset childrens' transform
	this->parent = parent;
}

GameObject* GameObject::GetParent()
{
	return parent;
}

void GameObject::DeleteMe()
{
	needToBeDeleted = true;

	parent->EraseChild(this);

	DeleteChildren();
}

void GameObject::AddChild(GameObject* children)
{
	this->children.push_back(children);
}

void GameObject::EraseChild(GameObject* child)
{
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

// EDIT
void GameObject::DeleteChild(uint index)
{
	children[index]->DeleteMe();
	children.erase(children.begin() + index);
}

void GameObject::DeleteChildren()
{
	for (int i = 0; i < children.size(); ++i)
		children[i]->DeleteMe();

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
	Component* newComponent;

	switch (type)
	{
	case No_type:
		break;
	case Mesh_Component:
		newComponent = App->renderer3D->CreateMeshComponent(this);
		break;
	case Material_Component:
		newComponent = new ComponentMaterial(this);
		break;
	default:
		break;
	}
	
	components.push_back(newComponent);
}

void GameObject::MarkToDeleteComponent(uint index)
{
	components[index]->needToBeDelated = true;
}

void GameObject::MarkToDeleteAllComponents()
{
	for (int i = 0; i < components.size(); ++i)
		components[i]->needToBeDelated = true;
}

void GameObject::InternallyDeleteComponent(uint index)
{
	delete components[index];
	components.erase(components.begin() + index);
}

void GameObject::InternallyDeleteComponents()
{
	for (int i = 0; i < components.size(); ++i)
		delete components[i];

	components.clear();
}

void GameObject::InternallyDeleteMarkedComponents()
{
	for (int i = 0; i < components.size(); ++i)
	{
		if (components[i]->needToBeDelated)
			InternallyDeleteComponent(i);
	}
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


