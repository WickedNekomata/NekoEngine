#include "ComponentRigidActor.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCollider.h"

#include "imgui\imgui.h"

ComponentRigidActor::ComponentRigidActor(GameObject* parent, ComponentTypes componentType) : Component(parent, componentType) {}

ComponentRigidActor::~ComponentRigidActor()
{
	App->physics->RemoveActor(*gActor);
	gActor->release();
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidActor::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::Checkbox("Use Gravity", &useGravity))
		SetUseGravity(useGravity);
	// TODO: interpolate & collision detection
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidActor::Update() {}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidActor::UpdateShape()
{
	physx::PxShape* gShape = nullptr;

	// Detach current shape
	uint nbShapes = gActor->getNbShapes();
	if (nbShapes > 0)
	{
		gActor->getShapes(&gShape, 1);

		if (gShape != nullptr)
			gActor->detachShape(*gShape);
		gShape = nullptr;
	}

	// Update current shape
	gShape = parent->collider->GetShape();

	// Attach current shape
	if (gShape != nullptr)
		gActor->attachShape(*gShape);
}

void ComponentRigidActor::UpdateTransform() const
{
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	gActor->setGlobalPose(physx::PxTransform(physx::PxMat44(globalMatrix.Transposed().ptr())));
}

void ComponentRigidActor::UpdateGameObjectTransform() const
{
	physx::PxTransform gTransform = gActor->getGlobalPose();
	math::Quat rotation(gTransform.q.x, gTransform.q.y, gTransform.q.z, gTransform.q.w);
	math::float3 position(gTransform.p.x, gTransform.p.y, gTransform.p.z);
	math::float4x4 globalMatrix(rotation, position);
	parent->transform->SetMatrixFromGlobal(globalMatrix);
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidActor::SetUseGravity(bool useGravity)
{
	this->useGravity = useGravity;
	gActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);
}

// ----------------------------------------------------------------------------------------------------

physx::PxRigidActor* ComponentRigidActor::GetActor() const
{
	return gActor;
}

// ----------------------------------------------------------------------------------------------------


void ComponentRigidActor::OnWake()
{
	CONSOLE_LOG(LogTypes::Normal, "OnWake", LogTypes::Normal);
}

void ComponentRigidActor::OnSleep()
{
	CONSOLE_LOG(LogTypes::Normal, "OnSleep", LogTypes::Normal);
}