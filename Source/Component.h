#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "ComponentTypes.h"
#include "parson\parson.h"
#include "Globals.h"
#include <mono/metadata/object.h>
#include "EventSystem.h"

class GameObject;

class Component
{
public:

	Component(GameObject* parent, ComponentTypes componentType = ComponentTypes::NoComponentType);
	virtual void Activate() {};
	virtual ~Component();

	virtual void Update();

	virtual void OnEditor();

	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	virtual void OnSystemEvent(System_Event event);

	void ToggleIsActive();
	bool IsActive() const;

	bool IsTreeActive();

	ComponentTypes GetType() const;

	void SetParent(GameObject* parent);
	GameObject* GetParent() const;

	uint GetSerializationBytes();

	void OnSave(char*& cursor);
	void OnLoad(char*& cursor);

	virtual void OnEnable() {}
	virtual void OnDisable() {}

	MonoObject* GetMonoComponent();
	inline void SetMonoComponent(uint32_t monoCompHandle) { this->monoCompHandle = monoCompHandle; };

private:

	virtual uint GetInternalSerializationBytes() = 0;
	virtual void OnInternalSave(char*& cursor) = 0;
	virtual void OnInternalLoad(char*& cursor) = 0;
	virtual void OnUniqueEditor();

protected:

	ComponentTypes componentType;
	GameObject* parent = nullptr;
	bool isActive = true;

	uint32_t monoCompHandle = 0u;

public:

	uint UUID = 0;


};

#endif
