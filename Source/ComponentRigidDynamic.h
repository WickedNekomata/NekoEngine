#ifndef __COMPONENT_RIGID_DYNAMIC_H__
#define __COMPONENT_RIGID_DYNAMIC_H__

#include "Component.h"
#include "ComponentRigidActor.h"

class ComponentRigidDynamic : public ComponentRigidActor
{
public:

	ComponentRigidDynamic(GameObject* parent);
	//ComponentRigidBody(const ComponentRigidBody& componentRigidBody);
	~ComponentRigidDynamic();

	void OnUniqueEditor();
	
	void Update();

	// Sets
	void SetDensity(float density);
	void UpdateMassAndInertia();
	void SetMass(float mass);
	void SetCMass(math::float3 cMass);
	void SetInertia(math::float3 inertia);
	void SetLinearDamping(float linearDamping);
	void SetAngularDamping(float angularDamping);
	void SetMaxLinearVelocity(float maxLinearVelocity);
	void SetMaxAngularVelocity(float maxAngularVelocity);
	void FreezePosition(bool x, bool y, bool z);
	void FreezeRotation(bool x, bool y, bool z);
	void SetIsKinematic(bool isKinematic);

	void SetLinearVelocity(math::float3 linearVelocity);
	void SetAngularVelocity(math::float3 angularVelocity);
	void AddForce(math::float3 force, physx::PxForceMode::Enum forceMode = physx::PxForceMode::Enum::eFORCE);
	void ClearForce() const;
	void AddTorque(math::float3 torque, physx::PxForceMode::Enum forceMode = physx::PxForceMode::Enum::eFORCE);
	void ClearTorque() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

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

	math::float3 linearVelocity = math::float3::zero;
	math::float3 angularVelocity = math::float3::zero;
	math::float3 force = math::float3::zero;
	physx::PxForceMode::Enum forceMode = physx::PxForceMode::Enum::eFORCE;
	math::float3 torque = math::float3::zero;
};

#endif