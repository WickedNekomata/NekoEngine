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

	void SetName(char* name);
	const char* GetName() const;

	void ToggleIsStatic();
	bool IsStatic() const;

	void SetSeenLastFrame(bool seenLastFrame);
	bool GetSeenLastFrame() const;

	void RecalculateBoundingBox();

	void OnSave(JSON_Object* file);
	void OnLoad(JSON_Object* file);

private:

	char* name = nullptr;
	std::vector<Component*> components;

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;

	bool isStatic = true;
	bool seenLastFrame = false;

public:

	bool enabled = true;

	ComponentTransform* transform = nullptr;
	ComponentMaterial* materialRenderer = nullptr;
	ComponentMesh* meshRenderer = nullptr;
	ComponentCamera* camera = nullptr;

	math::AABB boundingBox;

	uint UUID = 0;
};

#endif