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

	void ToggleKinematic() const;
	
	void SetMass(float mass) const;
	void SetLinearVelocity(math::float3 linearVelocity) const;
	void SetAngularDamping(float angularDamping) const;
	
	//void OnInternalSave(JSON_Object* file);
	//void OnLoad(JSON_Object* file);

private:

	bool isKinematic = false;
};

#endif