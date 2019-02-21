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

	// Sets
	void SetCenter(math::float3& center);
	void SetRadius(float radius);
	void SetHalfHeight(float halfHeight);
	void SetDirection(CapsuleDirection direction);

	// Gets
	physx::PxCapsuleGeometry GetCapsuleGeometry() const;

	uint GetInternalSerializationBytes();
	void OnInternalLoad(char*& cursor) {}
	void OnInternalSave(char*& cursor) {}
	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	float radius = 0.5f;
	float halfHeight = 0.5f;
	CapsuleDirection direction = CapsuleDirection::CapsuleDirectionYAxis;
};

#endif