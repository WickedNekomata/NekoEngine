#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "ComponentTypes.h"
#include "parson\parson.h"

#include "Globals.h"

class GameObject;

class Component
{
public:

	Component(GameObject* parent, ComponentType type = ComponentType::NoComponentType);
	virtual void Activate() {};
	virtual ~Component();

	void Update();

	virtual void OnEditor();

	void ToggleIsActive();
	bool IsActive() const;

	bool IsTreeActive();

	ComponentType GetType() const;

	void SetParent(GameObject* parent);
	GameObject* GetParent() const;

	virtual void OnSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file) {};

	virtual void OnEnable() {}
	virtual void OnDisable() {}

private:

	virtual void OnInternalSave(JSON_Object* file) {};
	virtual void OnUniqueEditor();
	
protected:

	ComponentType type;
	GameObject* parent = nullptr;
	bool isActive = true;

public:

	uint UUID = 0;
};

#endif