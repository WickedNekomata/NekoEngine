#include "ComponentRigidStatic.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "imgui\imgui.h"

ComponentRigidStatic::ComponentRigidStatic(GameObject* parent) : ComponentRigidBody(parent, ComponentTypes::RigidStaticComponent) 
{
	gActor = App->physics->CreateRigidStatic(PxTransform(PxIDENTITY()), *App->physics->CreateShape(PxSphereGeometry(1.0f), *App->physics->GetDefaultMaterial()));
	UpdateTransform();
}

ComponentRigidStatic::~ComponentRigidStatic() {}

void ComponentRigidStatic::Update() {}

void ComponentRigidStatic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("RigidStatic");
	ImGui::Spacing();

	ComponentRigidBody::OnUniqueEditor();
#endif
}