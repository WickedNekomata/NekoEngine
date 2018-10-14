#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"

#include <vector>

#include "ComponentTypes.h"

struct Component;

class GameObject
{
public:

	GameObject(char* name, GameObject* parent);
	virtual ~GameObject();

	void Update() const;

	inline void SetParent(GameObject* parent);
	void AddChild(GameObject* children);
	void EraseChild(uint index);
	void DeleteChild(uint index);
	void DeleteChildren();
	bool HasChildren() const;
	uint GetChildrenLength() const;
	GameObject* GetChild(uint index) const;

	void AddComponent(ComponentType type);
	void DeleteComponent(uint index);
	void DeleteComponents();
	bool HasComponents() const;
	uint GetComponenetsLength() const;
	Component* GetComponent(uint index) const;

	const char* GetName() const;
	void SetName(char* name);

private:

	char* name = nullptr;
	std::vector<Component*> components;

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;

public:
	bool enabled = true;
	bool needToBeDeleted = false;
};

#endif