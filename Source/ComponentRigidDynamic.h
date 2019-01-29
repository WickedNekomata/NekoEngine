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
	
	void SetMass(float mass) const;
	void SetLinearDamping(float linearDamping) const;
	void SetAngularDamping(float angularDamping) const;
	void SetIsKinematic(bool isKinematic) const;

	void SetLinearVelocity(math::float3 linearVelocity) const;
	void SetAngularVelocity(math::float3 angularVelocity) const;
	void AddForce(math::float3 force) const;
	void ClearForce() const;
	void AddTorque(math::float3 torque) const;
	void ClearTorque() const;

	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	float mass = 1.0f;
	float linearDamping = 0.0f;
	float angularDamping = 0.05f;
	bool isKinematic = false;

	math::float3 linearVelocity = math::float3::zero;
	math::float3 angularVelocity = math::float3::zero;
	math::float3 force = math::float3::zero;
	math::float3 torque = math::float3::zero;
};

#endif