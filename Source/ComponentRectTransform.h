#ifndef __COMPONENT_RECTTRANSFORM_H__
#define __COMPONENT_RECTTRANSFORM_H__

#include "Component.h"

#include "Globals.h"

class ComponentRectTransform : public Component
{
public:

	ComponentRectTransform(GameObject* parent, ComponentTypes componentType = ComponentTypes::RectTransformComponent);
	ComponentRectTransform(const ComponentRectTransform& componentRectTransform);
	~ComponentRectTransform();

	void Update();

	void OnEditor();

public:
	uint x = 0.0f;
	uint y = 0.0f;
	uint x_dist = 0.0f;
	uint y_dist = 0.0f;
};

#endif