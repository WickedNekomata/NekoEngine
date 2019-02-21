#include "ComponentRigidActor.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "ComponentCollider.h"

#include "PhysicsConstants.h"

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

void ComponentRigidActor::UpdateShape(physx::PxShape* shape) const
{
	bool attach = true;

	// Detach current shape
	uint nbShapes = gActor->getNbShapes();
	if (nbShapes > 0)
	{
		physx::PxShape* gShape = nullptr;
		gActor->getShapes(&gShape, 1);

		if (gShape == shape && shape != nullptr)
			attach = false;
		else
			gActor->detachShape(*gShape);
	}

	// Attach current shape
	if (shape == nullptr)
	{
		if (parent->boundingBox.IsFinite())
			shape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
		else
			shape = App->physics->CreateShape(physx::PxBoxGeometry(PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE, PhysicsConstants::GEOMETRY_HALF_SIZE), *App->physics->GetDefaultMaterial());
		assert(shape != nullptr);
	}

	if (attach)
		gActor->attachShape(*shape);
}

void ComponentRigidActor::UpdateTransform(math::float4x4& globalMatrix) const
{
	assert(globalMatrix.IsFinite());
	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::zero;
	globalMatrix.Decompose(position, rotation, scale);

	if (!position.IsFinite() || !rotation.IsFinite())
	{
		CONSOLE_LOG(LogTypes::Warning, "The rigid actor transform cannot be updated since the position or the rotation of the game object is infinite");
		return;
	}

	gActor->setGlobalPose(physx::PxTransform(physx::PxVec3(position.x, position.y, position.z), 
		physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));
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

uint ComponentRigidActor::GetInternalSerializationBytes()
{
	return 0;
}
