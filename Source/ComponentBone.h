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

	uint GetInternalSerializationBytes();
	bool Save(JSON_Object* component_obj) const;
	bool Load(const JSON_Object* component_obj);

	bool SetResource(uint resource);

	void OnInternalSave(char*& cursor) {}
	void OnInternalLoad(char*& cursor) {}

public:

	ComponentMesh* attached_mesh = nullptr;

	uint res = 0;

};

#endif // __COMPONENT_BONE_H__