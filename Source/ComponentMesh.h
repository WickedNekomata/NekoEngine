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

	void OnSave(JSON_Object* file);

public:

	Mesh* mesh = nullptr;
	PrimitiveCube* debugBoundingBox = nullptr;
};

#endif