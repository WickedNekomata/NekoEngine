#include "ComponentCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentCollider::ComponentCollider(GameObject* parent, ComponentTypes componentColliderType) : Component(parent, componentColliderType)
{
	gMaterial = App->physics->GetDefaultMaterial();
	assert(gMaterial != nullptr);
}

ComponentCollider::~ComponentCollider() {}

void ComponentCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Checkbox("Is Trigger", &isTrigger);

	// TODO: gMaterial (drag and drop)

	bool updateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Center"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CenterX", ImGuiDataType_Float, (void*)&center.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (center.x <= 0.0f)
			center.x = 0.0f;
		updateShape = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CenterY", ImGuiDataType_Float, (void*)&center.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (center.y <= 0.0f)
			center.y = 0.0f;
		updateShape = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##CenterZ", ImGuiDataType_Float, (void*)&center.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (center.z <= 0.0f)
			center.z = 0.0f;
		updateShape = true;
	}
#endif
}