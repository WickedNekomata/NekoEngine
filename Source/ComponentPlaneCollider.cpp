#include "ComponentPlaneCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleLayers.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

// Only for static actors

ComponentPlaneCollider::ComponentPlaneCollider(GameObject* parent) : ComponentCollider(parent, ComponentTypes::PlaneColliderComponent)
{
	EncloseGeometry();

	colliderType = ColliderTypes::PlaneCollider;

	// -----

	physx::PxShapeFlags shapeFlags = gShape->getFlags();
	isTrigger = shapeFlags & physx::PxShapeFlag::Enum::eTRIGGER_SHAPE && !(shapeFlags & physx::PxShapeFlag::eSIMULATION_SHAPE);
	participateInContactTests = shapeFlags & physx::PxShapeFlag::Enum::eSIMULATION_SHAPE;
	participateInSceneQueries = shapeFlags & physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE;
}

ComponentPlaneCollider::ComponentPlaneCollider(const ComponentPlaneCollider& componentPlaneCollider, GameObject* parent) : ComponentCollider(componentPlaneCollider, parent, ComponentTypes::PlaneColliderComponent)
{
	EncloseGeometry();

	colliderType = componentPlaneCollider.colliderType;

	SetIsTrigger(componentPlaneCollider.isTrigger);
	SetParticipateInContactTests(componentPlaneCollider.participateInContactTests);
	SetParticipateInSceneQueries(componentPlaneCollider.participateInSceneQueries);

	// -----

	SetNormal(componentPlaneCollider.normal);
	SetDistance(componentPlaneCollider.distance);

	SetCenter(componentPlaneCollider.center);
}

ComponentPlaneCollider::~ComponentPlaneCollider() {}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Plane Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ComponentCollider::OnUniqueEditor();

		ImGui::Text("Normal"); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalX", &normal.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalY", &normal.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##NormalZ", &normal.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetNormal(normal);
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Distance"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##PlaneDistance", &distance, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetDistance(distance);
		ImGui::PopItemWidth();
	}
#endif
}

uint ComponentPlaneCollider::GetInternalSerializationBytes()
{
	return ComponentCollider::GetInternalSerializationBytes() + 
		sizeof(math::float3) +
		sizeof(float);
}

void ComponentPlaneCollider::OnInternalSave(char*& cursor)
{
	ComponentCollider::OnInternalSave(cursor);

	size_t bytes = sizeof(math::float3);
	memcpy(cursor, &normal, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &distance, bytes);
	cursor += bytes;
}

void ComponentPlaneCollider::OnInternalLoad(char*& cursor)
{
	ComponentCollider::OnInternalLoad(cursor);

	size_t bytes = sizeof(math::float3);
	memcpy(&normal, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&distance, cursor, bytes);
	cursor += bytes;

	// -----

	EncloseGeometry();
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::EncloseGeometry()
{
	RecalculateShape();
}

void ComponentPlaneCollider::RecalculateShape()
{
	ClearShape();

	physx::PxPlaneGeometry gPlaneGeometry;
	gShape = App->physics->CreateShape(gPlaneGeometry, *gMaterial);
	assert(gShape != nullptr);

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);

	Layer* layer = App->layers->GetLayer(parent->GetLayer());
	SetFiltering(layer->GetFilterGroup(), layer->GetFilterMask());

	// -----

	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(gShape);
}

// ----------------------------------------------------------------------------------------------------

void ComponentPlaneCollider::SetNormal(const math::float3& normal)
{
	assert(normal.IsFinite());
	this->normal = normal;

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);
}

void ComponentPlaneCollider::SetDistance(float distance)
{
	this->distance = distance;

	if (normal.IsZero())
	{
		CONSOLE_LOG(LogTypes::Warning, "The plane transform cannot be updated since the normal is zero");
		return;
	}

	physx::PxTransform relativePose = physx::PxTransformFromPlaneEquation(physx::PxPlane(normal.x, normal.y, normal.z, distance));
	gShape->setLocalPose(relativePose);
}

// ----------------------------------------------------------------------------------------------------

physx::PxPlaneGeometry ComponentPlaneCollider::GetPlaneGeometry() const
{
	physx::PxPlaneGeometry planeGeometry;
	gShape->getPlaneGeometry(planeGeometry);
	return planeGeometry;
}