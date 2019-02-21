#ifndef __COMPONENT_SPHERE_COLLIDER_H__
#define __COMPONENT_SPHERE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

class ComponentSphereCollider : public ComponentCollider
{
public:

	ComponentSphereCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentSphereCollider();

	void OnUniqueEditor();

	void RecalculateShape();

	// Sets
	void SetRadius(float radius);

	// Gets
	physx::PxSphereGeometry GetSphereGeometry() const;

	uint GetInternalSerializationBytes();
	void OnInternalLoad(char*& cursor) {}
	void OnInternalSave(char*& cursor) {}
	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	float radius = 0.5f;
};

#endif