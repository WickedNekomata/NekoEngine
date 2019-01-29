#ifndef __COMPONENT_RIGID_ACTOR_H__
#define __COMPONENT_RIGID_ACTOR_H__

#include "Component.h"

#include "physx/include/PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class ComponentRigidActor : public Component
{
public:

	ComponentRigidActor(GameObject* parent, ComponentTypes componentType);
	//ComponentRigidActor(const ComponentRigidActor& componentRigidActor);
	virtual ~ComponentRigidActor();

	virtual void OnUniqueEditor();

	void UpdateShape();
	void UpdateTransform() const;
	void UpdateGameObjectTransform() const;

	physx::PxRigidActor* GetActor() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	float mass = 1.0f;
	float drag = 0.0f;
	float angularDrag = 0.05f;
	bool useGravity = false;

	physx::PxRigidActor* gActor = nullptr;
};

#endif