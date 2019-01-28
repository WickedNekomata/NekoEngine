#include "ComponentRigidDynamic.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "imgui\imgui.h"

ComponentRigidDynamic::ComponentRigidDynamic(GameObject* parent) : ComponentRigidBody(parent, ComponentTypes::RigidDynamicComponent) 
{
	gActor = App->physics->CreateRigidDynamic(PxTransform(PxIDENTITY()), *App->physics->CreateShape(PxSphereGeometry(1.0f), *App->physics->GetDefaultMaterial()), 10.0f);
	UpdateTransform();
}

ComponentRigidDynamic::~ComponentRigidDynamic() 
{

}

void ComponentRigidDynamic::Update() {}

void ComponentRigidDynamic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("RigidDynamic");
	ImGui::Spacing();

	if (ImGui::Checkbox("Kinematic", &isKinematic))
		ToggleKinematic();

	ComponentRigidBody::OnUniqueEditor();
#endif
}

void ComponentRigidDynamic::ToggleKinematic() const
{
	gActor->is<PxRigidBody>()->setRigidBodyFlag(PxRigidBodyFlag::Enum::eKINEMATIC, isKinematic);
}