#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Globals.h"

#include <vector>

struct Component;
enum ComponentType;

class GameObject
{
public:

	GameObject(char* name, GameObject* parent);
	virtual ~GameObject();

	void Update() const;

	inline void SetParent(GameObject* parent);
	void Addchildren(GameObject* children);
	void EraseChild(uint index);
	void DeleteChild(uint index);
	void DeleteChildren();

	void AddComponent(ComponentType type);
	void DeleteComponent(uint index);
	void DeleteComponents();

	inline const char* GetName() const;
	inline void SetName(char* name);

private:

	char* name = nullptr;
	std::vector<Component*> components;

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;
};

#endif