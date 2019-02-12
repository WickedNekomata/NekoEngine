#ifndef __COMPONENT_RECT_H__
#define __COMPONENT_RECT_H__

#include "Component.h"

#include "Globals.h"

class ComponentRect : public Component
{
public:

	ComponentRect(GameObject* parent, ComponentTypes componentType = ComponentTypes::RectComponent);
	ComponentRect(const ComponentRect& componentTransform);
	~ComponentRect();

	void Update();

	void OnEditor();

public:
	float x = 0.0f;
	float y = 0.0f;
	float x_dist = 0.0f;
	float y_dist = 0.0f;
};

#endif