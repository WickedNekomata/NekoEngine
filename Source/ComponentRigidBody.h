#ifndef __COMPONENT_RIGID_BODY_H__
#define __COMPONENT_RIGID_BODY_H__

#include "Component.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"

using namespace physx;

enum GeometryTypes
{
	Sphere,
	Capsule,
	Box
};

class ComponentRigidBody : public Component
{
public:

	ComponentRigidBody(GameObject* parent, GeometryTypes geometryType);
	//ComponentRigidBody(const ComponentRigidBody& componentRigidBody);
	~ComponentRigidBody();

	void Update();

	void OnUniqueEditor();

	void CreateGeometry();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	GeometryTypes geometryType;

	math::float3 center = math::float3::zero;
	float radius = 0.0f;
	float halfHeight = 0.0f;
};

#endif