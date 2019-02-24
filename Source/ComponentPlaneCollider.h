#ifndef __COMPONENT_PLANE_COLLIDER_H__
#define __COMPONENT_PLANE_COLLIDER_H__

#include "Component.h"
#include "ComponentCollider.h"

class ComponentPlaneCollider : public ComponentCollider
{
public:

	ComponentPlaneCollider(GameObject* parent);
	ComponentPlaneCollider(const ComponentPlaneCollider& componentPlaneCollider);
	~ComponentPlaneCollider();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void EncloseGeometry();
	void RecalculateShape();

	// Sets
	void SetNormal(math::float3& normal);
	void SetDistance(float distance);

	// Gets
	physx::PxPlaneGeometry GetPlaneGeometry() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	math::float3 normal = math::float3(0.0f, 1.0f, 0.0f);
	float distance = 0.0f;
};

#endif