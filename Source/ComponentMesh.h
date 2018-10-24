#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

#include "Globals.h"

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	virtual ~ComponentMesh();

	void Update() const;

	virtual void OnUniqueEditor();

public:
	float* vertices = nullptr;
	uint verticesID = 0;
	uint indicesID = 0;
	uint indicesSize = 0;
	uint textCoordsID = 0;
};

#endif