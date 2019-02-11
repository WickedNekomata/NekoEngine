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

	ImGui::Text("Normal"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##NormalX", &normal.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetNormal(normal);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##NormalY", &normal.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetNormal(normal);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##NormalZ", &normal.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetNormal(normal);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Distance"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##PlaneDistance", &distance, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetDistance(distance);
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::RecalculateShape()
{
	ClearShape();

	physx::PxPlaneGeometry gPlaneGeometry;
	gShape = App->physics->CreateShape(gPlaneGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::SetNormal(math::float3& normal)
{
	assert(normal.IsFinite());
	this->normal = normal;

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);
}

void ComponentPlaneCollider::SetDistance(float distance)
{
	this->distance = distance;

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);
}

// ----------------------------------------------------------------------------------------------------

physx::PxPlaneGeometry ComponentPlaneCollider::GetPlaneGeometry() const
{
	physx::PxPlaneGeometry planeGeometry;
	gShape->getPlaneGeometry(planeGeometry);
	return planeGeometry;
}