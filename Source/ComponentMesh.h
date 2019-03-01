#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "Component.h"

#include "Globals.h"

struct Mesh;
class ResourceMesh;

class ComponentMesh : public Component
{
public:

	ComponentMesh(GameObject* parent);
	ComponentMesh(const ComponentMesh& componentMesh, GameObject* parent, bool include = true);
	~ComponentMesh();

	void Update();

	void SetResource(uint res_uuid);

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	uint res = 0u;

	// navmesh utility
	bool nv_walkable = true;

	uint root_bones_uid = 0u;

	uint root_bone = 0u;
};

#endif