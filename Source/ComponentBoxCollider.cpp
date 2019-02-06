#include "ComponentBoxCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentBoxCollider::ComponentBoxCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::BoxColliderComponent)
{
	RecalculateShape();

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentBoxCollider::~ComponentBoxCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentBoxCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Box Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	ImGui::Text("Half size"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeX", &halfSize.x, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetHalfSize(halfSize);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeY", &halfSize.y, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetHalfSize(halfSize);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##HalfSizeZ", &halfSize.z, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetHalfSize(halfSize);
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentBoxCollider::RecalculateShape()
{
	ClearShape();

	physx::PxBoxGeometry gBoxGeometry(halfSize.x, halfSize.y, halfSize.z);
	gShape = App->physics->CreateShape(gBoxGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->layer);
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// ----------

	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentBoxCollider::SetHalfSize(math::float3& halfSize)
{
	this->halfSize = halfSize;
	gShape->setGeometry(physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z));
}

// ----------------------------------------------------------------------------------------------------

physx::PxBoxGeometry ComponentBoxCollider::GetBoxGeometry() const
{
	physx::PxBoxGeometry boxGeometry;
	gShape->getBoxGeometry(boxGeometry);
	return boxGeometry;
}