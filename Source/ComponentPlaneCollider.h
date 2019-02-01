#ifndef __COMPONENT_PLANE_COLLIDER_H__
#define __COMPONENT_PLANE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

#include "physx/include/PxPhysicsAPI.h"

class ComponentPlaneCollider : public ComponentCollider
{
public:

	ComponentPlaneCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentPlaneCollider();

	void OnUniqueEditor();

	void RecalculateShape();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	math::float3 halfSize = math::float3(0.5f, 0.5f, 0.5f);
};

#endif