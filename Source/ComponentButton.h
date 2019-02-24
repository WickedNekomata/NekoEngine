#ifndef __COMPONENT_BUTTON_H__
#define __COMPONENT_BUTTON_H__

#include "Component.h"
#include <string>

enum UIState;

class ComponentButton : public Component
{
public:
	ComponentButton(GameObject * parent, ComponentTypes componentType = ComponentTypes::ButtonComponent);
	ComponentButton(const ComponentButton & componentButton);
	
	~ComponentButton();
	
	void Update();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	bool MouseInScreen(const uint * rect) const;

private:

	UIState state;

	std::string input;
	uint scancode;
};

#endif

