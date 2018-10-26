#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

#include "Globals.h"

struct Mesh;

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	virtual ~ComponentMesh();

	void Update() const;

	virtual void OnUniqueEditor();

public:

	Mesh* mesh = nullptr;
};

#endif