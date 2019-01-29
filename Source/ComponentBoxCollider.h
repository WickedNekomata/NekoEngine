#ifndef __COMPONENT_BOX_COLLIDER_H__
#define __COMPONENT_BOX_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

#include "physx/include/PxPhysicsAPI.h"

class ComponentBoxCollider : public ComponentCollider
{
public:

	ComponentBoxCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentBoxCollider();

	void OnUniqueEditor();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	math::float3 halfSize = math::float3(0.5f, 0.5f, 0.5f);
};

#endif