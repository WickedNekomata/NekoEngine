#include "ComponentBoxCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentBoxCollider::ComponentBoxCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::BoxColliderComponent)
{
	RecalculateShape();
}

ComponentBoxCollider::~ComponentBoxCollider() {}

void ComponentBoxCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Box Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool recalculateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Half size"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeX", ImGuiDataType_Float, (void*)&halfSize.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeY", ImGuiDataType_Float, (void*)&halfSize.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeZ", ImGuiDataType_Float, (void*)&halfSize.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;

	if (recalculateShape)
		RecalculateShape();
#endif
}

void ComponentBoxCollider::RecalculateShape()
{
	ClearShape();

	math::float3 hS = halfSize;
	hS = math::Abs(hS);

	physx::PxBoxGeometry gBoxGeometry(hS.x, hS.y, hS.z);
	gShape = App->physics->CreateShape(gBoxGeometry, *gMaterial);
	if (gShape == nullptr)
		return;

	// ----------

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);
}