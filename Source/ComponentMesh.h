#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	virtual ~ComponentMesh();

	void Update() const;

	virtual void OnEditor() const;
};

#endif