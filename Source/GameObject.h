#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"
#include "ComponentTypes.h"

#include "MathGeoLib/include/Geometry/AABB.h"

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
	virtual ~GameObject();

	void Update();

	void SetParent(GameObject* parent);
	GameObject* GetParent();
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
	void MarkToDeleteAllComponents();
	void InternallyDeleteComponent(Component* index);
	void InternallyDeleteComponents();
	bool HasComponents() const;
	uint GetComponenetsLength() const;
	Component* GetComponent(uint index) const;

	const char* GetName() const;
	void SetName(char* name);

	bool GetIsStatic() const;
	void ToggleIsStatic();

	void RecalculateBoundingBox();

private:

	char* name = nullptr;
	bool isStatic = true;
	std::vector<Component*> components;

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;

public:

	bool enabled = true;

	ComponentTransform* transform = nullptr;
	ComponentMaterial* materialRenderer = nullptr;
	ComponentMesh* meshRenderer = nullptr;
	ComponentCamera* camera = nullptr;

	math::AABB boundingBox;
};

#endif