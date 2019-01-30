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

ComponentCollider::~ComponentCollider() 
{
	ClearShape();

	gMaterial = nullptr;
}

void ComponentCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Checkbox("Is Trigger", &isTrigger);

	// TODO: gMaterial (drag and drop)

	bool recalculateShape = false;

	ImGui::Text("Center"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterX", &center.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterY", &center.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterZ", &center.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();

	if (recalculateShape)
		RecalculateShape();
#endif
}

void ComponentCollider::ClearShape()
{
	if (gShape != nullptr)
		gShape->release();
	gShape = nullptr;
}

physx::PxShape* ComponentCollider::GetShape() const
{
	return gShape;
}