#include "ComponentRigidDynamic.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"

#include "imgui\imgui.h"

ComponentRigidDynamic::ComponentRigidDynamic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidDynamicComponent)
{
	physx::PxShape* gShape = nullptr;
	if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), *App->physics->GetDefaultMaterial());

	gActor = App->physics->CreateRigidDynamic(physx::PxTransform(physx::PxIDENTITY()), *gShape, 10.0f);
	if (gActor == nullptr)
		return;

	// ----------

	if (parent->collider != nullptr)
		UpdateShape();
	UpdateTransform();
}

ComponentRigidDynamic::~ComponentRigidDynamic() {}

void ComponentRigidDynamic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Rigid Dynamic");
	ImGui::Spacing();

	if (ImGui::Checkbox("Kinematic", &isKinematic))
		ToggleKinematic();

	ComponentRigidActor::OnUniqueEditor();
#endif
}

void ComponentRigidDynamic::ToggleKinematic() const
{
	gActor->is<physx::PxRigidBody>()->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, isKinematic);
}

void ComponentRigidDynamic::SetMass(float mass) const
{
	gActor->is<physx::PxRigidDynamic>()->setMass(mass);
}

void ComponentRigidDynamic::SetLinearVelocity(math::float3 linearVelocity) const
{
	gActor->is<physx::PxRigidDynamic>()->setLinearVelocity(physx::PxVec3(linearVelocity.x, linearVelocity.y, linearVelocity.z));
}

void ComponentRigidDynamic::SetAngularDamping(float angularDamping) const
{
	gActor->is<physx::PxRigidDynamic>()->setAngularDamping(angularDamping);
}