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

	if (parent->cmp_collider != nullptr)
		gShape = parent->cmp_collider->GetShape();
	else if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
	assert(gShape != nullptr);

	gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
	assert(gActor != nullptr);

	rigidActorType = RigidActorTypes::RigidStatic;

	gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	UpdateTransform(globalMatrix);

	// -----

	physx::PxActorFlags actorFlags = gActor->getActorFlags();
	useGravity = !(actorFlags & physx::PxActorFlag::eDISABLE_GRAVITY);
}

ComponentRigidStatic::ComponentRigidStatic(const ComponentRigidStatic& componentRigidStatic) : ComponentRigidActor(componentRigidStatic, ComponentTypes::RigidStaticComponent)
{
	physx::PxShape* gShape = nullptr;

	if (componentRigidStatic.parent->cmp_collider != nullptr)
		gShape = componentRigidStatic.parent->cmp_collider->GetShape();
	else if (componentRigidStatic.parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(componentRigidStatic.parent->boundingBox.HalfSize().x, componentRigidStatic.parent->boundingBox.HalfSize().y, componentRigidStatic.parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
	assert(gShape != nullptr);

	gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
	assert(gActor != nullptr);

	rigidActorType = componentRigidStatic.rigidActorType;

	gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	UpdateTransform(globalMatrix);

	// -----

	SetUseGravity(componentRigidStatic.useGravity);
}

ComponentRigidStatic::~ComponentRigidStatic() {}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::OnUniqueEditor() 
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Rigid Static", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//ComponentRigidActor::OnUniqueEditor();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidStatic::Update()
{
	if (useGravity)
		UpdateGameObjectTransform();
}

uint ComponentRigidStatic::GetInternalSerializationBytes()
{
	return ComponentRigidActor::GetInternalSerializationBytes();
}

void ComponentRigidStatic::OnInternalSave(char*& cursor)
{
	ComponentRigidActor::OnInternalSave(cursor);
}

void ComponentRigidStatic::OnInternalLoad(char*& cursor)
{
	ComponentRigidActor::OnInternalLoad(cursor);
}

// ----------------------------------------------------------------------------------------------------