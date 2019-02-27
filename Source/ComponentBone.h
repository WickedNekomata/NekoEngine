#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"

class ComponentMesh;

class ComponentBone : public Component
{
public:

	ComponentBone(GameObject* embedded_game_object);
	ComponentBone(GameObject* embedded_game_object, uint resource);
	~ComponentBone();

	void OnEditor();

	uint GetInternalSerializationBytes();

	bool SetResource(uint resource);

	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

public:
	ComponentMesh* attached_mesh = nullptr;

	uint res = 0u;
	uint attachedMesh = 0u;
};

#endif // __COMPONENT_BONE_H__