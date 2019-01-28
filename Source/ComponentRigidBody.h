#ifndef __COMPONENT_RIGID_BODY_H__
#define __COMPONENT_RIGID_BODY_H__

#include "Component.h"

#include "Geometry.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"

using namespace physx;

class ComponentRigidBody : public Component
{
public:

	ComponentRigidBody(GameObject* parent, ComponentTypes componentType);
	//ComponentRigidBody(const ComponentRigidBody& componentRigidBody);
	~ComponentRigidBody();

	void Update();

	void OnUniqueEditor();

	void ResetCurrentGeometry() const;
	void UpdateCurrentShape();

	void SetTransform(float* ptr) const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	PxRigidActor* gActor = nullptr;
	Geometry* geometry = nullptr;
};

#endif