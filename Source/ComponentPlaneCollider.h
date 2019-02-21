#ifndef __COMPONENT_PLANE_COLLIDER_H__
#define __COMPONENT_PLANE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

class ComponentPlaneCollider : public ComponentCollider
{
public:

	ComponentPlaneCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentPlaneCollider();

	void OnUniqueEditor();

	void RecalculateShape();

	// Sets
	void SetNormal(math::float3& normal);
	void SetDistance(float distance);

	// Gets
	physx::PxPlaneGeometry GetPlaneGeometry() const;

	uint GetInternalSerializationBytes();
	void OnInternalLoad(char*& cursor) {}
	void OnInternalSave(char*& cursor) {}
	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	math::float3 normal = math::float3(1.0f, 0.0f, 0.0f);
	float distance = 0.0f;
};

#endif