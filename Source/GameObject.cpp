#include "GameObject.h"

#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include <algorithm>

GameObject::GameObject(char* name, GameObject* parent) : name(name), parent(parent)
{
	AddComponent(ComponentType::Transform_Component);
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
	App->GOs->SetToDelete(this);

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
	case Transform_Component:
		newComponent = new ComponentTransform(this);
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
	App->GOs->SetComponentToDelete(components[index]);
}

void GameObject::MarkToDeleteAllComponents()
{
	for (int i = 0; i < components.size(); ++i)
		App->GOs->SetComponentToDelete(components[i]);
}

void GameObject::InternallyDeleteComponent(Component* toDelete)
{
	switch (toDelete->GetType())
	{
	case ComponentType::Mesh_Component:
		App->renderer3D->EraseComponent((ComponentMesh*)toDelete);
		break;
	}

	delete toDelete;
	components.erase(std::remove(components.begin(), components.end(), toDelete), components.end());
}

void GameObject::InternallyDeleteComponents()
{
	for (int i = 0; i < components.size(); ++i)
	{   
		switch (components[i]->GetType())
		{
		case ComponentType::Mesh_Component:
			App->renderer3D->EraseComponent((ComponentMesh*)components[i]);
			break;
		}		
		delete components[i];
	}
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


