#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"

class ComponentAnimation : public Component
{
public:

	ComponentAnimation(GameObject* embedded_game_object);
	ComponentAnimation(GameObject* embedded_game_object, uint resource);
	ComponentAnimation(const ComponentAnimation& component_anim, GameObject* parent, bool include = true);
	~ComponentAnimation();

	uint GetInternalSerializationBytes();
	bool SetResource(uint resource);

	bool PlayAnimation(const char* anim_name);

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);
	void OnEditor();
	void OnUniqueEditor();

public:
	uint res = 0;

};

#endif // __COMPONENT_ANIMATION_H__