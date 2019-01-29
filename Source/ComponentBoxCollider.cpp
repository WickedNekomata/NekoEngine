#include "ComponentBoxCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentBoxCollider::ComponentBoxCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::BoxColliderComponent)
{
	physx::PxBoxGeometry gBoxGeometry(halfSize.x, halfSize.y, halfSize.z);
	gShape = App->physics->CreateShape(gBoxGeometry, *gMaterial);
	assert(gShape != nullptr);
}

ComponentBoxCollider::~ComponentBoxCollider() {}

void ComponentBoxCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Box Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool updateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Half size"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeX", ImGuiDataType_Float, (void*)&halfSize.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (halfSize.x <= 0.0f)
			halfSize.x = 0.0f;
		updateShape = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeY", ImGuiDataType_Float, (void*)&halfSize.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (halfSize.y <= 0.0f)
			halfSize.y = 0.0f;
		updateShape = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##HalfSizeZ", ImGuiDataType_Float, (void*)&halfSize.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (halfSize.z <= 0.0f)
			halfSize.z = 0.0f;
		updateShape = true;
	}
#endif
}