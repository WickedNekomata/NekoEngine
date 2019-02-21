#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Layers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentCapsuleCollider::ComponentCapsuleCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::CapsuleColliderComponent)
{
	RecalculateShape();

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentCapsuleCollider::~ComponentCapsuleCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Capsule Collider");
	ImGui::Spacing();

	ComponentCollider::OnUniqueEditor();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Radius"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);	
	if (ImGui::DragFloat("##CapsuleRadius", &radius, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetRadius(radius);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Half height"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CapsuleHalfHeight", &halfHeight, 0.01f, 0.01f, FLT_MAX, "%.2f", 1.0f))
		SetHalfHeight(halfHeight);
	ImGui::PopItemWidth();

	const char* capsuleDirection[] = { "X-Axis", "Y-Axis", "Z-Axis" };
	int currentCapsuleDirection = direction;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Direction", &currentCapsuleDirection, capsuleDirection, IM_ARRAYSIZE(capsuleDirection)))
	{
		direction = (CapsuleDirection)currentCapsuleDirection;
		SetDirection(direction);
	}
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::RecalculateShape()
{
	ClearShape();

	physx::PxCapsuleGeometry gCapsuleGeometry(radius, halfHeight);
	gShape = App->physics->CreateShape(gCapsuleGeometry, *gMaterial);
	assert(gShape != nullptr);

	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionXAxis:
	{
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionYAxis:
	{
		math::float3 dir = math::float3(0.0f, 0.0f, 1.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionZAxis:
	{
		math::float3 dir = math::float3(0.0f, 1.0f, 0.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	}

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::SetCenter(math::float3& center)
{
	assert(center.IsFinite());
	this->center = center;
	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionXAxis:
	{
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionYAxis:
	{
		math::float3 dir = math::float3(0.0f, 0.0f, 1.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionZAxis:
	{
		math::float3 dir = math::float3(0.0f, 1.0f, 0.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	}
}

void ComponentCapsuleCollider::SetRadius(float radius)
{
	this->radius = radius;
	gShape->setGeometry(physx::PxCapsuleGeometry(radius, halfHeight));
}

void ComponentCapsuleCollider::SetHalfHeight(float halfHeight)
{
	this->halfHeight = halfHeight;
	gShape->setGeometry(physx::PxCapsuleGeometry(radius, halfHeight));
}

void ComponentCapsuleCollider::SetDirection(CapsuleDirection direction)
{
	this->direction = direction;
	switch (direction)
	{
	case CapsuleDirection::CapsuleDirectionXAxis:
	{
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionYAxis:
	{
		math::float3 dir = math::float3(0.0f, 0.0f, 1.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	case CapsuleDirection::CapsuleDirectionZAxis:
	{
		math::float3 dir = math::float3(0.0f, 1.0f, 0.0f);
		physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(dir.x, dir.y, dir.z)));
		gShape->setLocalPose(relativePose);
	}
	break;
	}
}

// ----------------------------------------------------------------------------------------------------

physx::PxCapsuleGeometry ComponentCapsuleCollider::GetCapsuleGeometry() const
{
	physx::PxCapsuleGeometry capsuleGeometry;
	gShape->getCapsuleGeometry(capsuleGeometry);
	return capsuleGeometry;
}

uint ComponentCapsuleCollider::GetInternalSerializationBytes()
{
	return 0;
}
