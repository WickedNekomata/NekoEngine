#ifndef __COMPONENT_BUTTON_H__
#define __COMPONENT_BUTTON_H__

#include "Component.h"
#include <string>

#include <mono/metadata/class.h>

enum UIState;

class ComponentButton : public Component
{
public:
	ComponentButton(GameObject * parent, ComponentTypes componentType = ComponentTypes::ButtonComponent);
	ComponentButton(const ComponentButton & componentButton, GameObject* parent);
	
	~ComponentButton();
	
	//NOTE: If you override this method, make sure to call the base class method. 
	//(Component::OnSystemEvent(event); at start)
	void OnSystemEvent(System_Event event);

	void Update();

	void KeyPressed();
	UIState GetState()const;

	void SetNewKey(uint key);

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	bool MouseInScreen(const uint* rect) const;

private:

	UIState state;

	std::string input;
	uint button_blinded;

	void SetNewKey(const char* key);

	MonoMethod* methodToCall = nullptr;
	MonoObject* scriptInstance = nullptr;

	GameObject* draggedGO = nullptr;
};

#endif

