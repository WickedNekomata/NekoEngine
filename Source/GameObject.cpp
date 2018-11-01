#include "GameObject.h"

#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"

#include "MathGeoLib/include/Geometry/OBB.h"
#include "PCG/pcg_variants.h"

#include <algorithm>

GameObject::GameObject(char* name, GameObject* parent) : parent(parent)
{
	this->name = new char[DEFAULT_BUF_SIZE];
	strcpy_s(this->name, DEFAULT_BUF_SIZE, name);

	if (parent != nullptr)
		AddComponent(ComponentType::Transform_Component);

	boundingBox.SetNegativeInfinity();

	if (isStatic)
		App->scene->quadtree.Insert(this);

	UUID = pcg32_random_r(&(App->rng));
}

GameObject::~GameObject()
{
	RELEASE_ARRAY(name);
	InternallyDeleteComponents();

	if (isStatic)
	{
		// Recreate the quadtree (game object deleted)
		App->scene->RecreateQuadtree();
	}
}

void GameObject::Update() {}

void GameObject::SetParent(GameObject* parent)
{
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

void GameObject::DeleteChild(uint index)
{
	children[index]->DeleteMe();
	children.erase(children.begin() + index);
}

void GameObject::DeleteChildren()
{
	for (uint i = 0; i < children.size();)
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

Component* GameObject::AddComponent(ComponentType type)
{
	Component* newComponent;

	switch (type)
	{
	case No_type:
		break;
	case Transform_Component:
		newComponent = transform = new ComponentTransform(this);
		break;
	case Mesh_Component:
		newComponent = meshRenderer = App->renderer3D->CreateMeshComponent(this);
		break;
	case Material_Component:
		newComponent = materialRenderer = new ComponentMaterial(this);
		break;
	case Camera_Component:
		newComponent = camera = App->renderer3D->CreateCameraComponent(this);
		break;
	default:
		break;
	}
	
	components.push_back(newComponent);

	return newComponent;
}

void GameObject::MarkToDeleteComponent(uint index)
{
	App->GOs->SetComponentToDelete(components[index]);
}

void GameObject::MarkToDeleteAllComponents()
{
	for (uint i = 0; i < components.size(); ++i)
		App->GOs->SetComponentToDelete(components[i]);
}

void GameObject::InternallyDeleteComponent(Component* toDelete)
{
	switch (toDelete->GetType())
	{
	case ComponentType::Mesh_Component:
		App->renderer3D->EraseMeshComponent((ComponentMesh*)toDelete);
		break;
	case ComponentType::Camera_Component:
		App->renderer3D->EraseCameraComponent((ComponentCamera*)toDelete);
		break;
	}

	RELEASE(toDelete);
	components.erase(std::remove(components.begin(), components.end(), toDelete), components.end());
}

void GameObject::InternallyDeleteComponents()
{
	for (int i = components.size() - 1; i >= 0; --i)
	{   
		switch (components[i]->GetType())
		{
		case ComponentType::Mesh_Component:
			App->renderer3D->EraseMeshComponent((ComponentMesh*)components[i]);
			break;
		case ComponentType::Camera_Component:
			App->renderer3D->EraseCameraComponent((ComponentCamera*)components[i]);
			break;
		}		

		RELEASE(components[i]);
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

void GameObject::SetName(char* name)
{
	strcpy_s(this->name, DEFAULT_BUF_SIZE, name);
}

const char* GameObject::GetName() const
{
	return name;
}

void GameObject::ToggleIsStatic()
{
	isStatic = !isStatic;

	if (isStatic)
		App->scene->quadtree.Insert(this);
	else
		// Recreate the quadtree (static changed)
		App->scene->RecreateQuadtree();
}

bool GameObject::IsStatic() const
{
	return isStatic;
}

void GameObject::SetSeenLastFrame(bool seenLastFrame)
{
	this->seenLastFrame = seenLastFrame;
}

bool GameObject::GetSeenLastFrame() const
{
	return seenLastFrame;
}

void GameObject::RecalculateBoundingBox()
{
	boundingBox.SetNegativeInfinity();

	// Grow bounding box
	if (meshRenderer != nullptr)
		boundingBox.Enclose((const math::float3*)meshRenderer->mesh->vertices, meshRenderer->mesh->verticesSize);

	// Transform bounding box (calculate OBB)
	math::OBB obb = boundingBox.Transform(transform->GetGlobalMatrix());

	// Calculate AABB
	if (obb.IsFinite())
		boundingBox = obb.MinimalEnclosingAABB();

	if (isStatic)
		// Recreate the quadtree (bounding box changed)
		App->scene->RecreateQuadtree();

	for (uint i = 0; i < children.size(); ++i)
		children[i]->RecalculateBoundingBox();
}

void GameObject::OnSave(JSON_Object* file)
{
	json_object_set_string(file, "name", name);
	json_object_set_number(file, "UUID", UUID);
	json_object_set_number(file, "Parent UUID", parent->UUID);

	JSON_Value* arrayValue = json_value_init_array();
	JSON_Array* jsonComponents = json_value_get_array(arrayValue);
	for (int i = 0; i < components.size(); ++i)
	{
		JSON_Value* newValue = json_value_init_object();
		JSON_Object* objToSerialize = json_value_get_object(newValue);

		components[i]->OnSave(objToSerialize);
		json_array_append_value(jsonComponents, newValue);	
	}
	json_object_set_value(file, "jsonComponents", arrayValue);
}

void GameObject::OnLoad(JSON_Object* file)
{
	UUID = json_object_get_number(file, "UUID");
	JSON_Array* jsonComponents = json_object_get_array(file, "jsonComponents");
	JSON_Object* cObject;

	for (int i = 0; i < json_array_get_count(jsonComponents); i++) {
		cObject = json_array_get_object(jsonComponents, i);
		Component* newComponent = AddComponent((ComponentType)(int)json_object_get_number(cObject, "Type"));
		newComponent->OnLoad(cObject);
	}	
}
