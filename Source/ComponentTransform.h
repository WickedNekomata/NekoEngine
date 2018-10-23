#ifndef __COMPONENT_TRANSFORM_H__
#define __COMPONENT_TRANSFORM_H__

#include "Component.h"

class ComponentTransform : public Component
{
public:

	ComponentTransform(GameObject* parent);
	virtual ~ComponentTransform();

	void Update() const;

	virtual void OnUniqueEditor() const;

private:

};

#endif