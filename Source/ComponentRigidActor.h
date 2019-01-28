#ifndef __COMPONENT_RIGID_ACTOR_H__
#define __COMPONENT_RIGID_ACTOR_H__

#include "Component.h"

#include "Geometry.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

using namespace physx;

class ComponentRigidActor : public Component
{
public:

	ComponentRigidActor(GameObject* parent, ComponentTypes componentType);
	//ComponentRigidActor(const ComponentRigidActor& componentRigidActor);
	~ComponentRigidActor();

	void Update();

	void OnUniqueEditor();

	void ResetGeometry() const;
	void UpdateShape();
	void UpdateTransform() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	PxRigidActor* gActor = nullptr;
	Geometry* geometry = nullptr;
};

#endif