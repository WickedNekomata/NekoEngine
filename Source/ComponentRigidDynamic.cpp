#include "ComponentRigidDynamic.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentRigidDynamic::ComponentRigidDynamic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidDynamicComponent)
{
	float minPointRadius = (parent->boundingBox.minPoint - parent->transform->position).Length();
	float maxPointRadius = (parent->boundingBox.maxPoint - parent->transform->position).Length();
	float radius = maxPointRadius >= minPointRadius ? maxPointRadius : minPointRadius;
	physx::PxShape* gShape = App->physics->CreateShape(physx::PxSphereGeometry(radius / 2.0f), *App->physics->GetDefaultMaterial());

	gActor = App->physics->CreateRigidDynamic(physx::PxTransform(physx::PxIDENTITY()), *gShape, 10.0f);
	//UpdateTransform();
}

ComponentRigidDynamic::~ComponentRigidDynamic() {}

void ComponentRigidDynamic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("RigidDynamic");
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