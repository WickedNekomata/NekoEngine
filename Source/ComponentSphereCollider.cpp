#include "ComponentSphereCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentSphereCollider::ComponentSphereCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::SphereColliderComponent)
{
	PxSphereGeometry gSphereGeometry(radius);
	gShape = App->physics->CreateShape(gSphereGeometry, *gMaterial);
	assert(gShape != nullptr);
}

ComponentSphereCollider::~ComponentSphereCollider() {}

void ComponentSphereCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Sphere Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool updateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SphereRadius", ImGuiDataType_Float, (void*)&radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
	{
		if (radius <= 0.0f)
			radius = 0.0f;
		updateShape = true;
	}
#endif
}