#ifndef __COMPONENT_RIGID_ACTOR_H__
#define __COMPONENT_RIGID_ACTOR_H__

#include "Component.h"

#include "physx\include\PxPhysicsAPI.h"

#include "MathGeoLib\include\Math\float4x4.h"

enum RigidActorTypes
{
	NoRigidActor,
	RigidStatic,
	RigidDynamic
};

class ComponentRigidActor : public Component
{
public:

	ComponentRigidActor(GameObject* parent, ComponentTypes componentRigidActorType);
	ComponentRigidActor(const ComponentRigidActor& componentRigidActor, ComponentTypes componentRigidActorType);
	virtual ~ComponentRigidActor();

	virtual void OnUniqueEditor();

	virtual void Update();

	virtual uint GetInternalSerializationBytes() = 0;
	virtual void OnInternalSave(char*& cursor) = 0;
	virtual void OnInternalLoad(char*& cursor) = 0;

	// ----------------------------------------------------------------------------------------------------

	void UpdateShape(physx::PxShape* shape) const;
	void UpdateTransform(math::float4x4& globalMatrix) const;
	void UpdateGameObjectTransform() const;

	// Sets
	void SetUseGravity(bool useGravity);

	// Gets
	physx::PxRigidActor* GetActor() const;
	RigidActorTypes GetRigidActorType() const;

	// Callbacks
	void OnWake();
	void OnSleep();

protected:

	bool useGravity = true;

	// -----

	physx::PxRigidActor* gActor = nullptr;
	RigidActorTypes rigidActorType = RigidActorTypes::NoRigidActor;
};

#endif