#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentCapsuleCollider::ComponentCapsuleCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::CapsuleColliderComponent)
{
	RecalculateShape();
}

ComponentCapsuleCollider::~ComponentCapsuleCollider() {}

void ComponentCapsuleCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Capsule Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool recalculateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CapsuleRadius", ImGuiDataType_Float, (void*)&radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Half height"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CapsuleHalfHeight", ImGuiDataType_Float, (void*)&halfHeight, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;

	const char* capsuleDirection[] = { "X-Axis", "Y-Axis", "Z-Axis" };
	int currentCapsuleDirection = direction;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Direction", &currentCapsuleDirection, capsuleDirection, IM_ARRAYSIZE(capsuleDirection)))
	{
		direction = (CapsuleDirection)currentCapsuleDirection;
		recalculateShape = true;
	}

	if (recalculateShape)
		RecalculateShape();
#endif
}

void ComponentCapsuleCollider::RecalculateShape()
{
	ClearShape();

	float r = radius;
	r = std::abs(r);
	float hH = halfHeight;
	hH = std::abs(hH);

	physx::PxCapsuleGeometry gCapsuleGeometry(r, hH);
	gShape = App->physics->CreateShape(gCapsuleGeometry, *gMaterial);
	if (gShape == nullptr)
		return;

	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionXAxis:
	{
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
		gShape->setLocalPose(relativePose);
	}
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
}