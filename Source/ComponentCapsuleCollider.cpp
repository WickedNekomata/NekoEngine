#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentCapsuleCollider::ComponentCapsuleCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::CapsuleColliderComponent)
{
	RecalculateShape();

	Layer* layer = App->layers->GetLayer(parent->layer);
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());
}

ComponentCapsuleCollider::~ComponentCapsuleCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Capsule Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool recalculateShape = false;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);	
	if (ImGui::DragFloat("##CapsuleRadius", &radius, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Half height"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CapsuleHalfHeight", &halfHeight, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();

	const char* capsuleDirection[] = { "X-Axis", "Y-Axis", "Z-Axis" };
	int currentCapsuleDirection = direction;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Direction", &currentCapsuleDirection, capsuleDirection, IM_ARRAYSIZE(capsuleDirection)))
	{
		direction = (CapsuleDirection)currentCapsuleDirection;
		recalculateShape = true;
	}
	ImGui::PopItemWidth();

	if (recalculateShape)
		RecalculateShape();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::RecalculateShape()
{
	ClearShape();

	physx::PxCapsuleGeometry gCapsuleGeometry(radius, halfHeight);
	gShape = App->physics->CreateShape(gCapsuleGeometry, *gMaterial);
	assert(gShape != nullptr);

	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionXAxis:
	{
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionYAxis:
	{
		math::float3 dir = math::float3(0.0f, 0.0f, 1.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionZAxis:
	{
		math::float3 dir = math::float3(0.0f, 1.0f, 0.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	}

	// ----------

	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

physx::PxCapsuleGeometry ComponentCapsuleCollider::GetCapsuleGeometry() const
{
	physx::PxCapsuleGeometry capsuleGeometry;
	gShape->getCapsuleGeometry(capsuleGeometry);
	return capsuleGeometry;
}