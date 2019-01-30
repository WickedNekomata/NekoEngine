#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "ComponentTypes.h"
#include "parson\parson.h"

#include "Globals.h"

class GameObject;

class Component
{
public:

	Component(GameObject* parent, ComponentTypes componentType = ComponentTypes::NoComponentType);
	virtual void Activate() {};
	virtual ~Component();

	virtual void Update();

	virtual void OnEditor();

	void ToggleIsActive();
	bool IsActive() const;

	ComponentTypes GetType() const;

	void SetParent(GameObject* parent);
	GameObject* GetParent() const;

	virtual void OnSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file) {};

private:

	virtual void OnInternalSave(JSON_Object* file) {};
	virtual void OnUniqueEditor();

private:

	bool isActive = true;

protected:

	ComponentTypes componentType;
	GameObject* parent = nullptr;

public:

	uint UUID = 0;
};

#endif