#include "ComponentRigidDynamic.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentPlaneCollider.h"

#include "PhysicsConstants.h"

#include "imgui\imgui.h"

ComponentRigidDynamic::ComponentRigidDynamic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidDynamicComponent)
{
	density = PhysicsConstants::DENSITY;

	physx::PxShape* gShape = nullptr;
	if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
	assert(gShape != nullptr);

	gActor = App->physics->CreateRigidDynamic(physx::PxTransform(physx::PxIDENTITY()), *gShape, density, isKinematic);
	assert(gActor != nullptr);

	gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
	if (parent->collider != nullptr)
	{
		UpdateShape(parent->collider->GetShape());
		UpdateMassAndInertia();
	}
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	UpdateTransform(globalMatrix);

	// -----

	physx::PxActorFlags actorFlags = gActor->getActorFlags();
	useGravity = !(actorFlags & physx::PxActorFlag::eDISABLE_GRAVITY);

	mass = gActor->is<physx::PxRigidDynamic>()->getMass();
	physx::PxVec3 gCMass = gActor->is<physx::PxRigidDynamic>()->getCMassLocalPose().p;
	cMass = math::float3(gCMass.x, gCMass.y, gCMass.z);
	physx::PxVec3 gInertia = gActor->is<physx::PxRigidDynamic>()->getMassSpaceInertiaTensor();
	inertia = math::float3(gInertia.x, gInertia.y, gInertia.z);
	linearDamping = gActor->is<physx::PxRigidDynamic>()->getLinearDamping();
	angularDamping = gActor->is<physx::PxRigidDynamic>()->getAngularDamping();
	maxLinearVelocity = gActor->is<physx::PxRigidDynamic>()->getMaxLinearVelocity();
	maxAngularVelocity = gActor->is<physx::PxRigidDynamic>()->getMaxAngularVelocity();
	physx::PxRigidDynamicLockFlags rigidDynamicLockFlags = gActor->is<physx::PxRigidDynamic>()->getRigidDynamicLockFlags();
	freezePosition[0] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
	freezePosition[1] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
	freezePosition[2] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
	freezeRotation[0] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
	freezeRotation[1] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
	freezeRotation[2] = rigidDynamicLockFlags & physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
	physx::PxRigidBodyFlags rigidBodyFlags = gActor->is<physx::PxRigidBody>()->getRigidBodyFlags();
	isKinematic = rigidBodyFlags & physx::PxRigidBodyFlag::Enum::eKINEMATIC;
}

ComponentRigidDynamic::~ComponentRigidDynamic() {}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Rigid Dynamic");
	ImGui::Spacing();

	ComponentRigidActor::OnUniqueEditor();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Density"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##Density", &density, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetDensity(density);
	ImGui::PopItemWidth();

	ImGui::PushID("DensityButton");
	if (ImGui::SmallButton("Update mass and inertia"))
		UpdateMassAndInertia();
	ImGui::PopID();

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Mass"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##Mass", &mass, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetMass(mass);
	ImGui::PopItemWidth();

	// -----

	ImGui::Text("Center of mass");

	ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CMassX", &cMass.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCMass(cMass);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CMassY", &cMass.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCMass(cMass);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CMassZ", &cMass.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCMass(cMass);
	ImGui::PopItemWidth();

	// -----

	ImGui::Text("Moment of inertia");

	ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##InertiaX", &inertia.x, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetInertia(inertia);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##InertiaY", &inertia.y, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetInertia(inertia);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##InertiaZ", &inertia.z, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetInertia(inertia);
	ImGui::PopItemWidth();

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Linear damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##LinearDamping", &linearDamping, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetLinearDamping(linearDamping);
	ImGui::PopItemWidth();

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Angular damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##AngularDamping", &angularDamping, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetAngularDamping(angularDamping);
	ImGui::PopItemWidth();

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max linear velocity"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##MaxLinearVelocity", &maxLinearVelocity, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetMaxLinearVelocity(maxLinearVelocity);
	ImGui::PopItemWidth();

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max angular velocity"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##MaxAngularVelocity", &maxAngularVelocity, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetMaxAngularVelocity(maxAngularVelocity);
	ImGui::PopItemWidth();

	// -----

	if (ImGui::TreeNodeEx("Constraints", ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::Text("Freeze Position");
		if (ImGui::Checkbox("X##FreezePositionX", &freezePosition[0]))
			FreezePosition(freezePosition[0], freezePosition[1], freezePosition[2]);
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##FreezePositionY", &freezePosition[1]))
			FreezePosition(freezePosition[0], freezePosition[1], freezePosition[2]);
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##FreezePositionZ", &freezePosition[2]))
			FreezePosition(freezePosition[0], freezePosition[1], freezePosition[2]);

		ImGui::Text("Freeze Rotation");
		if (ImGui::Checkbox("X##FreezeRotationX", &freezeRotation[0]))
			FreezeRotation(freezeRotation[0], freezeRotation[1], freezeRotation[2]);
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##FreezeRotationY", &freezeRotation[1]))
			FreezeRotation(freezeRotation[0], freezeRotation[1], freezeRotation[2]);
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##FreezeRotationZ", &freezeRotation[2]))
			FreezeRotation(freezeRotation[0], freezeRotation[1], freezeRotation[2]);

		ImGui::TreePop();
	}

	// -----

	if (ImGui::Checkbox("Is Kinematic", &isKinematic))
		SetIsKinematic(isKinematic);

	// --------------------------------------------------

	ImGui::Spacing();

	const ImVec4 disabledTextColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

	ImGui::Text("Linear velocity");

	physx::PxVec3 gCurrentLinearVelocity = gActor->is<physx::PxRigidDynamic>()->getLinearVelocity();
	math::float3 currentLinearVelocity = math::float3(gCurrentLinearVelocity.x, gCurrentLinearVelocity.y, gCurrentLinearVelocity.z);
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.x); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.y); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.z);

	ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##LinearVelocityX", &linearVelocity.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##LinearVelocityY", &linearVelocity.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##LinearVelocityZ", &linearVelocity.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::PopItemWidth();

	ImGui::PushID("LinearVelocityButton");
	if (ImGui::SmallButton("Apply"))
		SetLinearVelocity(linearVelocity);
	ImGui::PopID();

	// -----

	ImGui::Text("Angular velocity");

	physx::PxVec3 gCurrentAngularVelocity = gActor->is<physx::PxRigidDynamic>()->getAngularVelocity();
	math::float3 currentAngularVelocity = math::float3(gCurrentAngularVelocity.x, gCurrentAngularVelocity.y, gCurrentAngularVelocity.z);
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.x); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.y); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.z);

	ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##AngularVelocityX", &angularVelocity.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##AngularVelocityY", &angularVelocity.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##AngularVelocityZ", &angularVelocity.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::PopItemWidth();

	ImGui::PushID("AngularVelocityButton");
	if (ImGui::SmallButton("Apply"))
		SetAngularVelocity(angularVelocity);
	ImGui::PopID();

	// -----

	ImGui::Text("Force"); 
	
	ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##ForceX", &force.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##ForceY", &force.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##ForceZ", &force.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::PopItemWidth();

	ImGui::PushID("AddForceButton");
	if (ImGui::SmallButton("Apply"))
		AddForce(force);
	ImGui::PopID();

	ImGui::SameLine();

	ImGui::PushID("ClearForceButton");
	if (ImGui::SmallButton("Clear"))
		ClearForce();
	ImGui::PopID();

	// -----

	ImGui::Text("Torque"); 
	
	ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##TorqueX", &torque.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##TorqueY", &torque.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragFloat("##TorqueZ", &torque.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f);
	ImGui::PopItemWidth();

	ImGui::PushID("AddTorqueButton");
	if (ImGui::SmallButton("Apply"))
		AddTorque(torque);
	ImGui::PopID();

	ImGui::SameLine();

	ImGui::PushID("ClearTorqueButton");
	if (ImGui::SmallButton("Clear"))
		ClearTorque();
	ImGui::PopID();

	// -----

	const char* forceModes[] = { "Force", "Impulse", "Velocity change", "Acceleration" };
	// Force (default): continuous changes that are effected by mass
	// Acceleration: continuous changes that aren't effected by mass
	// Impulse: instant change that is effected by mass
	// Velocity change: instant change that is not effected by mass
	static int currentForceMode = forceMode;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Force mode", &currentForceMode, forceModes, IM_ARRAYSIZE(forceModes)))
		forceMode = (physx::PxForceMode::Enum)currentForceMode;
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::Update()
{
	if (!gActor->is<physx::PxRigidDynamic>()->isSleeping())
		UpdateGameObjectTransform();
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::SetDensity(float density)
{
	this->density = density;
}

// Sets the mass, center of mass, and inertia tensor
void ComponentRigidDynamic::UpdateMassAndInertia()
{
	physx::PxRigidBodyExt::updateMassAndInertia(*gActor->is<physx::PxRigidBody>(), density);

	mass = gActor->is<physx::PxRigidDynamic>()->getMass();
	physx::PxVec3 gCMass = gActor->is<physx::PxRigidDynamic>()->getCMassLocalPose().p;
	cMass = math::float3(gCMass.x, gCMass.y, gCMass.z);
	physx::PxVec3 gInertia = gActor->is<physx::PxRigidDynamic>()->getMassSpaceInertiaTensor();
	inertia = math::float3(gInertia.x, gInertia.y, gInertia.z);
}

void ComponentRigidDynamic::SetMass(float mass)
{
	// mass = 0.0f equals infinite mass
	// infinite mass: the linear velocity of the body cannot be changed by any constraints
	this->mass = mass;
	gActor->is<physx::PxRigidDynamic>()->setMass(mass);
}

void ComponentRigidDynamic::SetCMass(math::float3& cMass)
{
	assert(cMass.IsFinite());
	this->cMass = cMass;
	gActor->is<physx::PxRigidDynamic>()->setCMassLocalPose(physx::PxTransform(physx::PxVec3(cMass.x, cMass.y, cMass.z)));
}

void ComponentRigidDynamic::SetInertia(math::float3& inertia)
{
	assert(inertia.IsFinite());
	// inertia = math::float3(0.0f, 0.0f, 0.0f) equals infinite inertia
	this->inertia = inertia;
	gActor->is<physx::PxRigidDynamic>()->setMassSpaceInertiaTensor(physx::PxVec3(inertia.x, inertia.y, inertia.z));
}

void ComponentRigidDynamic::SetLinearDamping(float linearDamping)
{
	this->linearDamping = linearDamping;
	gActor->is<physx::PxRigidDynamic>()->setLinearDamping(linearDamping);
}

void ComponentRigidDynamic::SetAngularDamping(float angularDamping)
{
	this->angularDamping = angularDamping;
	gActor->is<physx::PxRigidDynamic>()->setAngularDamping(angularDamping);
}

void ComponentRigidDynamic::SetMaxLinearVelocity(float maxLinearVelocity)
{
	this->maxLinearVelocity = maxLinearVelocity;
	gActor->is<physx::PxRigidDynamic>()->setMaxLinearVelocity(maxLinearVelocity);
}

void ComponentRigidDynamic::SetMaxAngularVelocity(float maxAngularVelocity)
{
	this->maxAngularVelocity = maxAngularVelocity;
	gActor->is<physx::PxRigidDynamic>()->setMaxAngularVelocity(maxAngularVelocity);
}

void ComponentRigidDynamic::FreezePosition(bool x, bool y, bool z)
{
	freezePosition[0] = x;
	freezePosition[1] = y;
	freezePosition[2] = z;

	physx::PxRigidDynamicLockFlags flags;
	if (x)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
	if (y)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
	if (z)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;

	gActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags(flags);
}

void ComponentRigidDynamic::FreezeRotation(bool x, bool y, bool z)
{
	freezeRotation[0] = x;
	freezeRotation[1] = y;
	freezeRotation[2] = z;

	physx::PxRigidDynamicLockFlags flags;
	if (x)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
	if (y)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
	if (z)
		flags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;

	gActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags(flags);
}

void ComponentRigidDynamic::SetIsKinematic(bool isKinematic)
{
	this->isKinematic = isKinematic;
	gActor->is<physx::PxRigidBody>()->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, isKinematic);
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::SetLinearVelocity(math::float3& linearVelocity)
{
	assert(linearVelocity.IsFinite());
	this->linearVelocity = linearVelocity;
	gActor->is<physx::PxRigidDynamic>()->setLinearVelocity(physx::PxVec3(linearVelocity.x, linearVelocity.y, linearVelocity.z));
}

void ComponentRigidDynamic::SetAngularVelocity(math::float3& angularVelocity)
{
	assert(angularVelocity.IsFinite());
	this->angularVelocity = angularVelocity;
	gActor->is<physx::PxRigidDynamic>()->setAngularVelocity(physx::PxVec3(angularVelocity.x, angularVelocity.y, angularVelocity.z));
}

void ComponentRigidDynamic::AddForce(math::float3& force, physx::PxForceMode::Enum forceMode)
{
	assert(force.IsFinite());
	// f = m*a (force = mass * acceleration)
	this->force = force;
	this->forceMode = forceMode;

	gActor->is<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z), forceMode);
}

void ComponentRigidDynamic::ClearForce() const
{
	gActor->is<physx::PxRigidDynamic>()->clearForce();
}

void ComponentRigidDynamic::AddTorque(math::float3& torque, physx::PxForceMode::Enum forceMode)
{
	assert(torque.IsFinite());
	this->torque = torque;
	this->forceMode = forceMode;

	gActor->is<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), forceMode);
}

void ComponentRigidDynamic::ClearTorque() const
{
	gActor->is<physx::PxRigidDynamic>()->clearTorque();
}