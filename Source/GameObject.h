#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"
#include "ComponentTypes.h"

#include "MathGeoLib/include/Geometry/AABB.h"
#include "parson/parson.h"

#include <vector>

struct Component;
struct ComponentTransform;
struct ComponentMaterial;
struct ComponentMesh;
struct ComponentCamera;

class GameObject
{
public:

	GameObject(char* name, GameObject* parent);
	GameObject(const GameObject& gameObject);
	virtual ~GameObject();

	void Update();

	void SetParent(GameObject* parent);
	GameObject* GetParent();
	uint GetParentUUID() const;

	void DeleteMe();
	void AddChild(GameObject* children);
	void EraseChild(GameObject* child);
	void DeleteChild(uint index);
	void DeleteChildren();
	bool HasChildren() const;
	uint GetChildrenLength() const;
	GameObject* GetChild(uint index) const;

	Component* AddComponent(ComponentType type);
	void MarkToDeleteComponent(uint index);
	void MarkToDeleteComponentByValue(Component* component);
	void MarkToDeleteAllComponents();
	void InternallyDeleteComponent(Component* index);
	void InternallyDeleteComponents();
	bool HasComponents() const;
	uint GetComponenetsLength() const;
	Component* GetComponent(uint index) const;
	int GetComponentIndexOnComponents(Component* component) const;
	void SwapComponents(Component* firstComponent, Component* secondComponent);
	void ReorderComponents(Component* source, Component* target);

	void SetName(char* name);
	const char* GetName() const;

	uint GetUUID() const;

	void ToggleIsActive();
	bool IsActive() const;

	void ToggleIsStatic();
	bool IsStatic() const;

	void SetSeenLastFrame(bool seenLastFrame);
	bool GetSeenLastFrame() const;

	void RecursiveRecalculateBoundingBoxes();

	void OnSave(JSON_Object* file);
	void OnLoad(JSON_Object* file);

public:

	ComponentTransform* transform = nullptr;
	ComponentMaterial* materialRenderer = nullptr;
	ComponentMesh* meshRenderer = nullptr;
	ComponentCamera* camera = nullptr;

	math::AABB boundingBox;

private:

	char* name = nullptr;
	uint UUID = 0;

	std::vector<Component*> components;

	GameObject* parent = nullptr;
	uint parentUUID = 0;

	std::vector<GameObject*> children;

	bool isActive = true;
	bool isStatic = true;
	bool seenLastFrame = false;
};

#endif