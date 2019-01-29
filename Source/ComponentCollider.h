#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"

class ComponentCollider : public Component
{
public:

	ComponentCollider(GameObject* parent, ComponentTypes componentColliderType);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	virtual ~ComponentCollider();

	virtual void OnUniqueEditor();

	virtual void ClearShape();

	virtual void RecalculateShape() = 0;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

	physx::PxShape* GetShape() const;

protected:

	bool isTrigger = false;
	physx::PxMaterial* gMaterial = nullptr;
	math::float3 center = math::float3::zero;

	physx::PxShape* gShape = nullptr;
};

#endif