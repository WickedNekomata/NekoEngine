#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent);
	ComponentMaterial(const ComponentMaterial& componentMaterial);
	~ComponentMaterial();

	void Update();

	void SetResource(uint materialUuid);

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	uint res = 0;
};

#endif