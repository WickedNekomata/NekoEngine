#include "ComponentSphereCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

ComponentSphereCollider::ComponentSphereCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::SphereColliderComponent)
{
	RecalculateShape();
}

ComponentSphereCollider::~ComponentSphereCollider() {}

void ComponentSphereCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Sphere Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool recalculateShape = false;
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SphereRadius", ImGuiDataType_Float, (void*)&radius, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		recalculateShape = true;

	if (recalculateShape)
		RecalculateShape();
#endif
}

void ComponentSphereCollider::RecalculateShape()
{
	ClearShape();

	float r = radius;
	r = std::abs(r);

	physx::PxSphereGeometry gSphereGeometry(r);
	gShape = App->physics->CreateShape(gSphereGeometry, *gMaterial);
	if (gShape == nullptr)
		return;

	// ----------

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);
}