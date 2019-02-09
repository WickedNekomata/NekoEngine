#include "ComponentRigidStatic.h"

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

ComponentRigidStatic::ComponentRigidStatic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidStaticComponent)
{
	physx::PxShape* gShape = nullptr;
	if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
	assert(gShape != nullptr);

	gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
	assert(gActor != nullptr);

	gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
	if (parent->collider != nullptr)
		UpdateShape(parent->collider->GetShape());
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	UpdateTransform(globalMatrix);

	// -----

	physx::PxActorFlags actorFlags = gActor->getActorFlags();
	useGravity = !(actorFlags & physx::PxActorFlag::eDISABLE_GRAVITY);
}

ComponentRigidStatic::~ComponentRigidStatic() {}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Rigid Static");
	ImGui::Spacing();

	ComponentRigidActor::OnUniqueEditor();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::Update() 
{
	if (useGravity)
		UpdateGameObjectTransform();
}