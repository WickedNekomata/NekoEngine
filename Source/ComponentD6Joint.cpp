#include "ComponentD6Joint.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentRigidActor.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentD6Joint::ComponentD6Joint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::D6Joint;

}

ComponentD6Joint::ComponentD6Joint(const ComponentD6Joint& componentD6Joint) : ComponentJoint(componentD6Joint, ComponentTypes::D6JointComponent)
{

}

ComponentD6Joint::~ComponentD6Joint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentD6Joint::OnUniqueEditor()
{
#ifndef GAMEMODE

#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentD6Joint::Update()
{
	physx::PxD6Joint* D6Joint = (physx::PxD6Joint*)gJoint;
	//physx::PxJointLinearLimitPair(App->physics->GetTolerancesScale()); // linear hard limit pair
	//physx::PxJointLinearLimitPair() // linear soft limit pair

	//prismaticJoint->setPrismaticJointFlag(physx::PxPrismaticJointFlag::eLIMIT_ENABLED, true);
}

uint ComponentD6Joint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentD6Joint::OnInternalSave(char*& cursor)
{
}

void ComponentD6Joint::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------