#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent, ComponentType type = ComponentType::No_type);
	virtual ~ComponentMaterial();

	void Update() const;

	virtual void OnEditor() const;
};

#endif