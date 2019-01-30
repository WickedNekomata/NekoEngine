#include "ComponentRigidStatic.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"

#include "imgui\imgui.h"

ComponentRigidStatic::ComponentRigidStatic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidStaticComponent)
{
	physx::PxShape* gShape = nullptr;
	if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), *App->physics->GetDefaultMaterial());

	gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *gShape);
	if (gActor == nullptr)
		return;
	gActor->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);

	// ----------

	if (parent->collider != nullptr)
		UpdateShape();
	UpdateTransform();
}

ComponentRigidStatic::~ComponentRigidStatic() {}

void ComponentRigidStatic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Rigid Static");
	ImGui::Spacing();

	ComponentRigidActor::OnUniqueEditor();
#endif
}