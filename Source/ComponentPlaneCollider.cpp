#include "ComponentPlaneCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

// Only for static actors

ComponentPlaneCollider::ComponentPlaneCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::PlaneColliderComponent)
{
	RecalculateShape();

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
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
	assert(gShape != nullptr);

	Layer* layer = App->layers->GetLayer(parent->layer);
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// ----------

	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

physx::PxPlaneGeometry ComponentPlaneCollider::GetPlaneGeometry() const
{
	physx::PxPlaneGeometry planeGeometry;
	gShape->getPlaneGeometry(planeGeometry);
	return planeGeometry;
}