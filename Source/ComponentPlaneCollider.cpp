#include "ComponentPlaneCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

// Only for static actors

ComponentPlaneCollider::ComponentPlaneCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::PlaneColliderComponent)
{
	RecalculateShape();
}

ComponentPlaneCollider::~ComponentPlaneCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Plane Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::RecalculateShape()
{
	ClearShape();

	physx::PxPlaneGeometry gPlaneGeometry;
	gShape = App->physics->CreateShape(gPlaneGeometry, *gMaterial);
	if (gShape == nullptr)
		return;
}