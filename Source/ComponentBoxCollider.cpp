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

	ImGui::Text("Half size"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeX", &halfSize.x, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeY", &halfSize.y, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeZ", &halfSize.z, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();

	if (recalculateShape)
		RecalculateShape();
#endif
}

void ComponentBoxCollider::RecalculateShape()
{
	ClearShape();

	physx::PxBoxGeometry gBoxGeometry(halfSize.x, halfSize.y, halfSize.z);
	gShape = App->physics->CreateShape(gBoxGeometry, *gMaterial);
	if (gShape == nullptr)
		return;

	// ----------

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);
}