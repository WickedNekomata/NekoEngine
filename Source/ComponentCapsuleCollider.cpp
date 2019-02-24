#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleLayers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentCapsuleCollider::ComponentCapsuleCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::CapsuleColliderComponent)
{
	EncloseGeometry();

	colliderType = ColliderTypes::CapsuleCollider;

	// -----

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentCapsuleCollider::ComponentCapsuleCollider(const ComponentCapsuleCollider& componentCapsuleCollider) : ComponentCollider(componentCapsuleCollider, ComponentTypes::CapsuleColliderComponent)
{
	EncloseGeometry();

	colliderType = componentCapsuleCollider.colliderType;

	SetIsTrigger(componentCapsuleCollider.isTrigger);
	SetParticipateInContactTests(componentCapsuleCollider.participateInContactTests);
	SetParticipateInSceneQueries(componentCapsuleCollider.participateInSceneQueries);

	// -----

	SetRadius(componentCapsuleCollider.radius);
	SetHalfHeight(componentCapsuleCollider.halfHeight);
	SetDirection(componentCapsuleCollider.direction);

	SetCenter(componentCapsuleCollider.center);
}

ComponentCapsuleCollider::~ComponentCapsuleCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Capsule Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
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
	}
#endif
}

uint ComponentCapsuleCollider::GetInternalSerializationBytes()
{
	return ComponentCollider::GetInternalSerializationBytes() + 
		sizeof(float) +
		sizeof(float) +
		sizeof(CapsuleDirection);
}

void ComponentCapsuleCollider::OnInternalSave(char*& cursor)
{
	ComponentCollider::OnInternalSave(cursor);

	size_t bytes = sizeof(float);
	memcpy(cursor, &radius, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &halfHeight, bytes);
	cursor += bytes;

	bytes = sizeof(CapsuleDirection);
	memcpy(cursor, &direction, bytes);
	cursor += bytes;
}

void ComponentCapsuleCollider::OnInternalLoad(char*& cursor)
{
	ComponentCollider::OnInternalLoad(cursor);

	size_t bytes = sizeof(float);
	memcpy(&radius, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&halfHeight, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(CapsuleDirection);
	memcpy(&direction, cursor, bytes);
	cursor += bytes;

	// -----

	EncloseGeometry();
}

// ----------------------------------------------------------------------------------------------------

void ComponentCapsuleCollider::EncloseGeometry()
{
	math::float4x4 globalMatrix = parent->transform->GetGlobalMatrix();
	math::AABB boundingBox = parent->boundingBox;

	if (globalMatrix.IsFinite() && parent->boundingBox.IsFinite())
	{
		math::float3 pos = math::float3::zero;
		math::Quat rot = math::Quat::identity;
		math::float3 scale = math::float3::one;
		globalMatrix.Decompose(pos, rot, scale);

		assert(parent->boundingBox.IsFinite());
		center = parent->boundingBox.CenterPoint() - pos;
		center = globalMatrix.Float3x3Part().Inverted() * center;

		math::float3 halfSize = globalMatrix.Float3x3Part().Inverted() * parent->boundingBox.HalfSize();

		switch (direction)
		{
		case CapsuleDirection::CapsuleDirectionXAxis:

			radius = 0.5f * halfSize.x;
			halfHeight = 0.5f * halfSize.x;

			break;

		case CapsuleDirection::CapsuleDirectionYAxis:

			radius = 0.5f * halfSize.y;
			halfHeight = 0.5f * halfSize.y;

			break;

		case CapsuleDirection::CapsuleDirectionZAxis:

			radius = 0.5f * halfSize.z;
			halfHeight = 0.5f * halfSize.z;

			break;
		}
	}

	RecalculateShape();
}

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

void ComponentCapsuleCollider::SetCenter(const math::float3& center)
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