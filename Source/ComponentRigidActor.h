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

	void SetUseGravity(bool useGravity);

	physx::PxRigidActor* GetActor() const;

	void OnWake();
	void OnSleep();

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

protected:

	bool useGravity = true;

	physx::PxRigidActor* gActor = nullptr;
};

#endif