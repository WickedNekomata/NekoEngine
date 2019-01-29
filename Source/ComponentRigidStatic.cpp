#include "ComponentRigidStatic.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentRigidStatic::ComponentRigidStatic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidStaticComponent)
{
	gActor = App->physics->CreateRigidStatic(physx::PxTransform(physx::PxIDENTITY()), *App->physics->CreateShape(physx::PxSphereGeometry(parent->boundingBox.HalfDiagonal().Length()), *App->physics->GetDefaultMaterial()));
	//UpdateTransform();
}

ComponentRigidStatic::~ComponentRigidStatic() {}

void ComponentRigidStatic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("RigidStatic");
	ImGui::Spacing();

	ComponentRigidActor::OnUniqueEditor();
#endif
}