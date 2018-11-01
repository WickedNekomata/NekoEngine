#ifndef __COMPONENT_H__
#define __COMPONENT_H__

struct GameObject;

#include "ComponentTypes.h"
#include "parson/parson.h"

#include "Globals.h"

class Component
{
public:

	Component(GameObject* parent, ComponentType type = ComponentType::No_type);
	virtual ~Component();

	void Update();

	virtual void OnEditor();

	virtual ComponentType GetType();
	virtual GameObject* GetParent();

	virtual void OnSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file) = 0;

private:

	virtual void OnInternalSave(JSON_Object* file) = 0;
	virtual void OnUniqueEditor();

protected:

	ComponentType type;
	GameObject* parent = nullptr;

public:
	uint UUID = 0;
};

#endif