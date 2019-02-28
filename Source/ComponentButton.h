#ifndef __COMPONENT_BUTTON_H__
#define __COMPONENT_BUTTON_H__

#define B_STATE_IDLE 0
#define B_STATE_HOVERED 1
#define B_STATE_R_CLICK 2
#define B_STATE_L_CLICK 3

#include "Component.h"
#include <string>

enum UIState;

class ComponentButton : public Component
{
public:
	ComponentButton(GameObject * parent, ComponentTypes componentType = ComponentTypes::ButtonComponent);
	ComponentButton(const ComponentButton & componentButton, GameObject* parent);
	
	~ComponentButton();
	
	void Update();

	bool* GetFlags();

	void KeyPressed();

private:
	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();

	bool MouseInScreen(const uint * rect) const;

private:

	UIState state;

	std::string input;
	uint button_blinded;

	bool sate_flag[4] = { true, false, false, false };

	void SetNewKey(const char* key);
	void SetNewKey(uint key);
};

#endif

