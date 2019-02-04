#include "ComponentSphereCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

#include "MathGeoLib\include\Math\float4x4.h"

ComponentSphereCollider::ComponentSphereCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::SphereColliderComponent)
{
	if (parent->boundingBox.IsFinite())
	{
		math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
		math::float3 position = math::float3::zero;
		math::Quat rotation = math::Quat::identity;
		math::float3 scale = math::float3::one;
		globalMatrix.Decompose(position, rotation, scale);

		center = parent->boundingBox.CenterPoint() - position;
		radius = parent->boundingBox.HalfDiagonal().Length();
	}

	RecalculateShape();
}

ComponentSphereCollider::~ComponentSphereCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Sphere Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	bool recalculateShape = false;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##SphereRadius", &radius, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		recalculateShape = true;
	ImGui::PopItemWidth();

	if (recalculateShape)
		RecalculateShape();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphereCollider::RecalculateShape()
{
	ClearShape();

	physx::PxSphereGeometry gSphereGeometry(radius);
	gShape = App->physics->CreateShape(gSphereGeometry, *gMaterial);
	if (gShape == nullptr)
		return;

	// ----------

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);
}