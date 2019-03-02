#ifndef __COMPONENT_BOX_COLLIDER_H__
#define __COMPONENT_BOX_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

class ComponentBoxCollider : public ComponentCollider
{
public:

	ComponentBoxCollider(GameObject* parent);
	ComponentBoxCollider(const ComponentBoxCollider& componentBoxCollider, GameObject* parent);
	~ComponentBoxCollider();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void EncloseGeometry();
	void RecalculateShape();

	// Sets
	void SetHalfSize(const math::float3& halfSize);

	// Gets
	physx::PxBoxGeometry GetBoxGeometry() const;

private:

	math::float3 halfSize = math::float3(0.5f, 0.5f, 0.5f);
};

#endif