#include "ComponentPlaneCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "imgui\imgui.h"

// Only for static actors

ComponentPlaneCollider::ComponentPlaneCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::PlaneColliderComponent)
{
	RecalculateShape();

	Layer* layer = App->layers->GetLayer(parent->layer);
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());
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