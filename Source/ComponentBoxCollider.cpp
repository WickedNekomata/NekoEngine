#include "ComponentBoxCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleLayers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentBoxCollider::ComponentBoxCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::BoxColliderComponent)
{
	EncloseGeometry();

	colliderType = ColliderTypes::BoxCollider;

	// -----

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentBoxCollider::ComponentBoxCollider(const ComponentBoxCollider& componentBoxCollider) : ComponentCollider(componentBoxCollider, ComponentTypes::BoxColliderComponent)
{

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

uint ComponentBoxCollider::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentBoxCollider::OnInternalSave(char*& cursor)
{
}

void ComponentBoxCollider::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentBoxCollider::EncloseGeometry()
{
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	math::AABB boundingBox = parent->boundingBox;

	if (globalMatrix.IsFinite() && parent->boundingBox.IsFinite())
	{
		math::float3 pos = math::float3::zero;
		math::Quat rot = math::Quat::identity;
		math::float3 scale = math::float3::one;
		globalMatrix.Decompose(pos, rot, scale);

		center = parent->boundingBox.CenterPoint() - pos;
		center = globalMatrix.Float3x3Part().Inverted() * center;

		halfSize = globalMatrix.Float3x3Part().Inverted() * parent->boundingBox.HalfSize();
	}

	RecalculateShape();
}

void ComponentBoxCollider::RecalculateShape()
{
	ClearShape();

	physx::PxBoxGeometry gBoxGeometry(halfSize.x, halfSize.y, halfSize.z);
	gShape = App->physics->CreateShape(gBoxGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentBoxCollider::SetHalfSize(math::float3& halfSize)
{
	assert(halfSize.IsFinite());
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