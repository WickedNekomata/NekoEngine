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
	ComponentCapsuleCollider(const ComponentCapsuleCollider& componentCapsuleCollider, GameObject* parent);
	~ComponentCapsuleCollider();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void EncloseGeometry();
	void RecalculateShape();

	// Sets
	void SetCenter(const math::float3& center);
	void SetRadius(float radius);
	void SetHalfHeight(float halfHeight);
	void SetDirection(CapsuleDirection direction);

	// Gets
	physx::PxCapsuleGeometry GetCapsuleGeometry() const;

private:

	float radius = 0.5f;
	float halfHeight = 0.5f;
	CapsuleDirection direction = CapsuleDirection::CapsuleDirectionYAxis;
};

#endif