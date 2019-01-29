#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentCapsuleCollider::ComponentCapsuleCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::CapsuleColliderComponent)
{
	PxCapsuleGeometry gCapsuleGeometry(radius, halfHeight);
	gShape = App->physics->CreateShape(gCapsuleGeometry, *gMaterial);
	assert(gShape != nullptr);

	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionYAxis:
	{
		math::float3 dir = math::float3(0.0f, 0.0f, 1.0f);
		PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionZAxis:
	{
		math::float3 dir = math::float3(0.0f, 1.0f, 0.0f);
		PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	}
}

ComponentCapsuleCollider::~ComponentCapsuleCollider() {}

void ComponentCapsuleCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Capsule Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool updateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CapsuleRadius", ImGuiDataType_Float, (void*)&radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (radius <= 0.0f)
			radius = 0.0f;
		updateShape = true;
	}

	ImGui::Text("Half height"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CapsuleHalfHeight", ImGuiDataType_Float, (void*)&halfHeight, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (halfHeight <= 0.0f)
			halfHeight = 0.0f;
		updateShape = true;
	}

	const char* capsuleDirection[] = { "X-Axis", "Y-Axis", "Z-Axis" };
	int currentCapsuleDirection = direction;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Direction", &currentCapsuleDirection, capsuleDirection, IM_ARRAYSIZE(capsuleDirection)))
	{
		direction = (CapsuleDirection)currentCapsuleDirection;
		updateShape = true;
	}
#endif
}