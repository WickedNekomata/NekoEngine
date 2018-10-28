#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

#include "Globals.h"

struct Mesh;
class PrimitiveCube;

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	~ComponentMesh();

	void Update();

	void OnUniqueEditor();

	void GrowBoundingBox() const;

	void CreateDebugBoundingBox();

public:

	Mesh* mesh = nullptr;
	PrimitiveCube* debugBoundingBox = nullptr;
};

#endif