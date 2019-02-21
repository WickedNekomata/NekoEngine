#include "GameObject.h"

#include "Application.h"
#include "ModuleGOs.h"
#include "ModuleResourceManager.h"
#include "ModulePhysics.h"
#include "ResourceMesh.h"

#include "ComponentTypes.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentPlaneCollider.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidStatic.h"
#include "ComponentRigidDynamic.h"
#include "ComponentNavAgent.h"
#include "ComponentEmitter.h"
#include "ComponentBone.h"
#include "ComponentScript.h"

#include "MathGeoLib/include/Geometry/OBB.h"

GameObject::GameObject(const char* name, GameObject* parent, bool disableTransform) : parent(parent)
{
	strcpy_s(this->name, DEFAULT_BUF_SIZE, name);

	if (parent != nullptr)
	{
		parent->AddChild(this);
		parent_uuid = parent->uuid;

		if (!disableTransform)
			AddComponent(ComponentTypes::TransformComponent);
	}

	boundingBox.SetNegativeInfinity();

	uuid = App->GenerateRandomNumber();
}

GameObject::GameObject(const GameObject& gameObject)
{
	strcpy_s(name, DEFAULT_BUF_SIZE, gameObject.name);

	for (int i = 0; i < gameObject.components.size(); ++i)
	{
		ComponentTypes type = gameObject.components[i]->GetType();

		switch (type)
		{
		case ComponentTypes::TransformComponent:
			transform = new ComponentTransform(*gameObject.transform);
			transform->SetParent(this);
			components.push_back(transform);
			break;
		case ComponentTypes::MeshComponent:
			cmp_mesh = new ComponentMesh(*gameObject.cmp_mesh);
			cmp_mesh->SetParent(this);
			components.push_back(cmp_mesh);
			break;
		case ComponentTypes::MaterialComponent:
			cmp_material = new ComponentMaterial(*gameObject.cmp_material);
			cmp_mesh->SetParent(this);
			components.push_back(cmp_material);
			break;
		case ComponentTypes::CameraComponent:
			cmp_camera = new ComponentCamera(*gameObject.cmp_camera);
			cmp_camera->SetParent(this);
			components.push_back(cmp_camera);
			break;
		case ComponentTypes::NavAgentComponent:
			cmp_navAgent = new ComponentNavAgent(*gameObject.cmp_navAgent);
			cmp_navAgent->SetParent(this);
			components.push_back(cmp_navAgent);
			break;
		case ComponentTypes::EmitterComponent:
			cmp_emitter = new ComponentEmitter(*gameObject.cmp_emitter);
			cmp_emitter->SetParent(this);
			components.push_back(cmp_emitter);
			break;
		case ComponentTypes::RigidStaticComponent:
		case ComponentTypes::RigidDynamicComponent:
			// TODO
			cmp_rigidActor = App->physics->CreateRigidActorComponent(this, type);
			break;
		case ComponentTypes::BoxColliderComponent:
		case ComponentTypes::SphereColliderComponent:
		case ComponentTypes::CapsuleColliderComponent:
		case ComponentTypes::PlaneColliderComponent:
			// TODO
			cmp_collider = App->physics->CreateColliderComponent(this, type);
			break;
		}
	}

	boundingBox = gameObject.boundingBox;

	isActive = gameObject.isActive;
	isStatic = gameObject.isStatic;
	seenLastFrame = gameObject.seenLastFrame;

	uuid = App->GenerateRandomNumber();
	parent_uuid = gameObject.parent_uuid;
	parent = gameObject.parent;
}

GameObject::~GameObject()
{
	// Components could not be destroyed by event at fbx exportation, for example.
	for (int i = 0; i < components.size(); ++i)
		delete components[i];
}

void GameObject::SetName(const char* name)
{
	strcpy_s((char*)this->name, DEFAULT_BUF_SIZE, name);
}

const char* GameObject::GetName() const
{
	return name;
}

uint GameObject::GetUUID() const
{
	return uuid;
}

void GameObject::ForceUUID(uint uuid)
{
	this->uuid = uuid;
}

void GameObject::SetParent(GameObject* parent)
{
	this->parent = parent;
	parent_uuid = parent->GetUUID();
}

GameObject* GameObject::GetParent() const
{
	return parent;
}

uint GameObject::GetParentUUID() const
{
	return parent_uuid;
}

void GameObject::ToggleIsActive()
{
	isActive = !isActive;

	isActive ? OnEnable() : OnDisable();
}

void GameObject::ToggleIsStatic()
{
	isStatic = !isStatic;
	App->GOs->RecalculateVector(this);
}

bool GameObject::IsActive() const
{
	return isActive;
}

bool GameObject::IsStatic() const
{
	return isStatic;
}

void GameObject::OnEnable()
{
	for (int i = 0; i < components.size(); ++i)
		components[i]->OnEnable();

	for (int i = 0; i < children.size(); ++i)
		children[i]->OnEnable();
}

void GameObject::OnDisable()
{
	for (int i = 0; i < components.size(); ++i)
		components[i]->OnDisable();

	for (int i = 0; i < children.size(); ++i)
		children[i]->OnDisable();
}

void GameObject::RecursiveRecalculateBoundingBoxes()
{
	boundingBox.SetNegativeInfinity();

	// Grow bounding box
	if (cmp_mesh != nullptr && cmp_mesh->res != 0)
	{
		const ResourceMesh* meshRes = (const ResourceMesh*)App->res->GetResource(cmp_mesh->res);
		int nVerts = meshRes->GetVerticesCount();
		float* vertices = new float[nVerts * 3];
		meshRes->GetTris(vertices);
		boundingBox.Enclose((const math::float3*)vertices, nVerts);
		delete[] vertices;
	}

	// Transform bounding box (calculate OBB)
	math::OBB obb;
	obb.SetFrom(boundingBox);
	math::float4x4 transformMatrix = transform->GetGlobalMatrix();
	obb.Transform(transformMatrix);

	// Calculate AABB
	if (obb.IsFinite())
		boundingBox = obb.MinimalEnclosingAABB();

	for (uint i = 0; i < children.size(); ++i)
		children[i]->RecursiveRecalculateBoundingBoxes();
}

void GameObject::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::RecalculateBBoxes:
		RecursiveRecalculateBoundingBoxes();
		break;
	case System_Event_Type::ShaderProgramChanged:
		cmp_material->UpdateUniforms();
		break;
	}
}

void GameObject::Destroy()
{
	std::vector<GameObject*> toDestroy;

	GetChildrenAndThisVectorFromLeaf(toDestroy);

	bool recreateQuadtree = false;

	for (int i = 0; i < toDestroy.size(); ++i)
	{
		for (int j = 0; j < toDestroy[i]->components.size(); ++j)
		{
			System_Event newEvent;
			newEvent.compEvent.type = System_Event_Type::ComponentDestroyed;
			newEvent.compEvent.component = toDestroy[i]->components[j];
			App->PushSystemEvent(newEvent);
		}
		System_Event newEvent;
		newEvent.goEvent.type = System_Event_Type::GameObjectDestroyed;
		newEvent.goEvent.gameObject = toDestroy[i];
		App->PushSystemEvent(newEvent);

		if (toDestroy[i]->IsStatic())
			recreateQuadtree = true;
	}

	if (recreateQuadtree)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
	}
}

void GameObject::GetChildrenAndThisVectorFromLeaf(std::vector<GameObject*>& go)
{
	for (int i = 0; i < children.size(); ++i)
		children[i]->GetChildrenAndThisVectorFromLeaf(go);

	go.push_back(this);
}

void GameObject::AddChild(GameObject* target)
{
	children.push_back(target);
}

bool GameObject::EraseChild(const GameObject* child)
{
	if (std::find(children.begin(), children.end(), child) == children.end())
		return false;

	children.erase(std::remove(children.begin(), children.end(), child), children.end());

	return true;
}

bool GameObject::IsChild(GameObject* target, bool untilTheEnd = false) const
{
	bool ret = false;

	for (int i = 0; i < children.size() && !ret; ++i)
	{
		if (target == children[i])
			return true;

		if (untilTheEnd)
			ret = children[i]->IsChild(target, true);
	}

	return ret;
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

bool GameObject::EqualsToChildrenOrThis(const void* isEqual) const
{
	bool ret = false;

	if (isEqual == this)
		ret = true;

	for (int i = 0; i < children.size() && !ret; ++i)
		ret = children[i]->EqualsToChildrenOrThis(isEqual);

	return ret;
}

Component* GameObject::AddComponent(ComponentTypes componentType, bool createDependencies)
{
	Component* newComponent;
	Component* newMaterial = 0;

	switch (componentType)
	{
	case ComponentTypes::TransformComponent:
		assert(transform == NULL);
		newComponent = transform = new ComponentTransform(this);
		break;
	case ComponentTypes::MeshComponent:
		assert(cmp_mesh == NULL);
		newComponent = cmp_mesh = new ComponentMesh(this);
		if (createDependencies)
		{
			assert(cmp_material == NULL);
			newMaterial = cmp_material = new ComponentMaterial(this);
		}
		break;
	case ComponentTypes::MaterialComponent:
		assert(cmp_material == NULL);
		newComponent = cmp_material = new ComponentMaterial(this);
		break;
	case ComponentTypes::CameraComponent:
		assert(cmp_camera == NULL);
		newComponent = cmp_camera = new ComponentCamera(this);
		break;
	case ComponentTypes::NavAgentComponent:
		assert(cmp_navAgent == NULL);
		newComponent = cmp_navAgent = new ComponentNavAgent(this);
		break;
	case ComponentTypes::EmitterComponent:
		assert(cmp_emitter == NULL);
		newComponent = cmp_emitter = new ComponentEmitter(this);
		break;
	case ComponentTypes::RigidStaticComponent:
	case ComponentTypes::RigidDynamicComponent:
		// TODO
		assert(cmp_rigidActor == nullptr);
		newComponent = cmp_rigidActor = App->physics->CreateRigidActorComponent(this, componentType);
		break;
	case ComponentTypes::BoxColliderComponent:
	case ComponentTypes::SphereColliderComponent:
	case ComponentTypes::CapsuleColliderComponent:
	case ComponentTypes::PlaneColliderComponent:
		// TODO
		assert(cmp_collider == nullptr);
		newComponent = cmp_collider = App->physics->CreateColliderComponent(this, componentType);
		break;
	}

	components.push_back(newComponent);

	if (newMaterial)
		components.push_back(newMaterial);

	return newComponent;
}

void GameObject::AddComponent(Component* component)
{
	components.push_back(component);
}

bool GameObject::DestroyComponent(Component* destroyed)
{
	if (std::find(components.begin(), components.end(), destroyed) == components.end())
		return false;

	assert(destroyed->GetType() != ComponentTypes::MaterialComponent);

	System_Event newEvent;
	newEvent.compEvent.type = System_Event_Type::ComponentDestroyed;
	newEvent.compEvent.component = destroyed;
	App->PushSystemEvent(newEvent);

	if (destroyed->GetType() == ComponentTypes::MeshComponent)
	{
		assert(cmp_material != NULL);
		newEvent.compEvent.type = System_Event_Type::ComponentDestroyed;
		newEvent.compEvent.component = cmp_material;
		App->PushSystemEvent(newEvent);
	}
	return true;
}

void GameObject::EraseComponent(Component* erased)
{
	components.erase(std::remove(components.begin(), components.end(), erased), components.end());
}

Component* GameObject::GetComponent(int index) const
{
	return components[index];
}

Component* GameObject::GetComponent(ComponentTypes type) const
{
	Component* comp = 0;
	for (int i = 0; i < components.size(); ++i)
	{
		if (components[i]->GetType() == type)
			comp = components[i];
	}
	return comp;
}

int GameObject::GetComponentsLength()
{
	return components.size();
}

void GameObject::ReorderComponents(Component* source, Component* target)
{
	components.erase(std::remove(components.begin(), components.end(), target), components.end());
	int index;
	for (int i = 0; i < components.size(); ++i)
	{
		if (components[i] == target)
			index = i;
	}

	components.erase(std::remove(components.begin(), components.end(), source), components.end());
	components.insert(components.begin() + index, source);
}

void GameObject::GetChildrenVector(std::vector<GameObject*>& go)
{
	go.push_back(this);

	for (int i = 0; i < children.size(); i++)
		children[i]->GetChildrenVector(go);
}

uint GameObject::GetSerializationBytes() const
{				   // uuid + parent + layer + active + static + name + number of components
	size_t size = sizeof(uint) * 3 + sizeof(bool) * 2 + sizeof(char) * DEFAULT_BUF_SIZE + sizeof(int);

	for (int i = 0; i < components.size(); ++i)
		size += components[i]->GetSerializationBytes();

	return size;
}

void GameObject::OnSave(char*& cursor) const
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &parent_uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &layer, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &isActive, bytes);
	cursor += bytes;

	memcpy(cursor, &isStatic, bytes);
	cursor += bytes;

	bytes = sizeof(char) * DEFAULT_BUF_SIZE;
	memcpy(cursor, &name, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int totalCmps = components.size();
	memcpy(cursor, &totalCmps, bytes);
	cursor += bytes;

	for (int i = 0; i < components.size(); ++i)
		components[i]->OnSave(cursor);
}

void GameObject::OnLoad(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(&uuid, cursor, bytes);
	cursor += bytes;

	memcpy(&parent_uuid, cursor, bytes);
	cursor += bytes;

	memcpy(&layer, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&isActive, cursor, bytes);
	cursor += bytes;

	memcpy(&isStatic, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(char) * DEFAULT_BUF_SIZE;
	memcpy(&name, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int totalCmps;
	memcpy(&totalCmps, cursor, bytes);
	cursor += bytes;

	components.reserve(totalCmps);
	for (int i = 0; i < totalCmps; ++i)
	{
		bytes = sizeof(int);
		ComponentTypes componentType;
		memcpy(&componentType, cursor, bytes);
		cursor += bytes;
		Component* cmp = AddComponent(componentType, false);
		cmp->OnLoad(cursor);
	}
}

// review this
void GameObject::RecursiveForceAllResources(uint forceRes) const
{
	if (cmp_material != nullptr)
	{
		for (int i = 0; i < cmp_material->res.size(); ++i)
			cmp_material->res[i].res = forceRes;

		cmp_material->shaderProgramUUID = forceRes;
	}

	if (cmp_mesh != nullptr)
		cmp_mesh->res = forceRes;

	for (int i = 0; i < children.size(); ++i)
		children[i]->RecursiveForceAllResources(forceRes);
}

// Scripting
MonoObject* GameObject::GetMonoObject()
{
	return monoObjectHandle != 0 ? mono_gchandle_get_target(monoObjectHandle) : nullptr;
}

void GameObject::SetLayer(uint layerNumber)
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	if (layer == layerNumber)
		return;

	layer = layerNumber;

	System_Event newEvent;
	newEvent.type = System_Event_Type::LayerChanged;
	newEvent.layerEvent.layer = layerNumber;
	newEvent.layerEvent.collider = cmp_collider;
	App->PushSystemEvent(newEvent);
}

uint GameObject::GetLayer() const
{
	return layer;
}