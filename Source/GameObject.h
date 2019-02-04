#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"
#include "ComponentTypes.h"
#include "EventSystem.h"

#include "MathGeoLib\include\Geometry\AABB.h"
#include "parson\parson.h"

#include <vector>

#include <mono/metadata/object.h>

class Component;
class ComponentTransform;
class ComponentMaterial;
class ComponentMesh;
class ComponentCamera;
class ComponentEmitter;
class ComponentRigidActor;
class ComponentCollider;

class GameObject
{
public:

	GameObject(const char* name, GameObject* parent, bool disableTransform = false);
	GameObject(const GameObject& gameObject);
	void Activate() const;
	virtual ~GameObject();

	void Update();

	void OnSystemEvent(System_Event event);

	void SetParent(GameObject* parent);
	GameObject* GetParent() const;
	uint GetParentUUID() const;

	void DestroyChildren();
	void DeleteMe();
	void AddChild(GameObject* children);
	void EraseChild(GameObject* child);
	void DeleteChild(uint index);
	void DeleteChildren();
	bool HasChildren() const;
	uint GetChildrenLength() const;
	GameObject* GetChild(uint index) const;
	bool IsChild(const GameObject* target, bool untilTheEnd) const;

	void AddComponent(Component* component);
	void ClearComponent(Component* component);
	Component* AddComponent(ComponentTypes componentType);
	void MarkToDeleteComponent(uint index);
	void MarkToDeleteComponentByValue(Component* component);
	void MarkToDeleteAllComponents();
	void InternallyDeleteComponent(Component* index);
	void InternallyDeleteComponents();
	bool HasComponents() const;
	uint GetComponentsLength() const;
	Component* GetComponent(uint index) const;
	Component * GetComponentByType(ComponentTypes type) const;
	int GetComponentIndexOnComponents(Component* component) const;
	void SwapComponents(Component* firstComponent, Component* secondComponent);
	void ReorderComponents(Component* source, Component* target);

	bool EqualsToChildrenOrMe(const void* isEqual) const;

	void SetName(const char* name);
	const char* GetName() const;

	uint GetUUID() const;
	void ForceUUID(uint uuid);

	void ToggleIsActive();
	bool IsActive() const;

	void ToggleIsStatic();
	bool IsStatic() const;

	void SetSeenLastFrame(bool seenLastFrame);
	bool GetSeenLastFrame() const;

	MonoObject* GetMonoObject();
	inline void SetMonoObject(uint32_t monoObjectHandle) { this->monoObjectHandle = monoObjectHandle; };

	void RecursiveRecalculateBoundingBoxes();

	void OnSave(JSON_Object* file) const;
	void OnLoad(JSON_Object* file);
	void RecursiveSerialitzation(JSON_Array* goArray) const;

	void RecursiveForceAllResources(uint forceRes) const;

	void OnEnable();
	void OnDisable();

public:

	ComponentTransform* transform = nullptr;
	ComponentMaterial* materialRenderer = nullptr;
	ComponentMesh* meshRenderer = nullptr;
	ComponentCamera* camera = nullptr;
	ComponentEmitter* emitter = nullptr;

	// Physics
	ComponentRigidActor* rigidActor = nullptr;
	ComponentCollider* collider = nullptr;

	math::AABB boundingBox;

	// Layer
	uint layer = 0; // in the range [0...31]

private:

	const char* name = nullptr;
	uint UUID = 0;

	std::vector<Component*> components;

	GameObject* parent = nullptr;
	uint parentUUID = 0;

	std::vector<GameObject*> children;

	uint32_t monoObjectHandle = 0;

	bool isActive = true;
	bool isStatic = true;
	bool seenLastFrame = false;
};

#endif
