#ifndef __COMPONENT_CAPSULE_COLLIDER_H__
#define __COMPONENT_CAPSULE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

enum CapsuleDirection
{
	CapsuleDirectionXAxis,
	CapsuleDirectionYAxis,
	CapsuleDirectionZAxis
};

class ComponentCapsuleCollider : public ComponentCollider
{
public:

	ComponentCapsuleCollider(GameObject* parent);
	//ComponentCollider(const ComponentRigidActor& componentRigidActor);
	~ComponentCapsuleCollider();

	void OnUniqueEditor();

	void RecalculateShape();

	physx::PxCapsuleGeometry GetCapsuleGeometry() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	float radius = 0.5f;
	float halfHeight = 0.5f;
	CapsuleDirection direction = CapsuleDirection::CapsuleDirectionYAxis;
};

#endif