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

	virtual void OnInternalSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file);

	void OnGameMode();
	void OnEditorMode();

public:

	Mesh* mesh = nullptr;
};

#endif