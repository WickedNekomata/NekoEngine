#ifndef __COMPONENT_RIGID_DYNAMIC_H__
#define __COMPONENT_RIGID_DYNAMIC_H__

#include "Component.h"
#include "ComponentRigidActor.h"

class ComponentRigidDynamic : public ComponentRigidActor
{
public:

	ComponentRigidDynamic(GameObject* parent);
	ComponentRigidDynamic(const ComponentRigidDynamic& componentRigidDynamic, GameObject* parent);
	~ComponentRigidDynamic();

	void OnUniqueEditor();

	void Update();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	// Sets
	void SetDensity(float density);
	void UpdateMassAndInertia();
	void SetMass(float mass);
	void SetCMass(const math::float3& cMass);
	void SetInertia(const math::float3& inertia);
	void SetLinearDamping(float linearDamping);
	void SetAngularDamping(float angularDamping);
	void SetMaxLinearVelocity(float maxLinearVelocity);
	void SetMaxAngularVelocity(float maxAngularVelocity);
	void FreezePosition(bool x, bool y, bool z);
	void FreezeRotation(bool x, bool y, bool z);
	void SetIsKinematic(bool isKinematic);

	void SetLinearVelocity(math::float3& linearVelocity);
	void SetAngularVelocity(math::float3& angularVelocity);
	// Force (default): continuous changes that are effected by mass
	// Acceleration: continuous changes that aren't effected by mass
	// Impulse: instant change that is effected by mass
	// Velocity change: instant change that is not effected by mass
	void AddForce(math::float3& force, physx::PxForceMode::Enum forceMode = physx::PxForceMode::Enum::eFORCE);
	void ClearForce() const;
	void AddTorque(math::float3& torque, physx::PxForceMode::Enum forceMode = physx::PxForceMode::Enum::eFORCE);
	void ClearTorque() const;

	// Gets
	bool IsSleeping() const;

private:

	float density = 0.0f;
	float mass = 0.0f;
	math::float3 cMass = math::float3::zero;
	math::float3 inertia = math::float3::zero;
	float linearDamping = 0.0f;
	float angularDamping = 0.0f;
	float maxLinearVelocity = 0.0f;
	float maxAngularVelocity = 0.0f;
	bool freezePosition[3] = { false, false, false };
	bool freezeRotation[3] = { false, false, false };
	bool isKinematic = false;
};

#endif