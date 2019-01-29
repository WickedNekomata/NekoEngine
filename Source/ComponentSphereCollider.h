#ifndef __COMPONENT_SPHERE_COLLIDER_H__
#define __COMPONENT_SPHERE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

#include "physx/include/PxPhysicsAPI.h"

class ComponentSphereCollider : public ComponentCollider
{
public:

	ComponentSphereCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentSphereCollider();

	void OnUniqueEditor();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	float radius = 0.5f;
};

#endif