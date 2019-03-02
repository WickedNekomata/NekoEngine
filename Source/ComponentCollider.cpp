#include "ComponentCollider.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "EventSystem.h"

#include "ComponentRigidActor.h"

#include "imgui\imgui.h"

ComponentCollider::ComponentCollider(GameObject* parent, ComponentTypes componentColliderType) : Component(parent, componentColliderType)
{
	gMaterial = App->physics->GetDefaultMaterial();
	assert(gMaterial != nullptr);

	if (parent->cmp_rigidActor == nullptr)
		CONSOLE_LOG(LogTypes::Warning, "Component Collider: You need to create a Component Rigid Actor in order to use the collider");

	App->physics->AddColliderComponent(this);
}

ComponentCollider::ComponentCollider(const ComponentCollider& componentCollider, GameObject* parent, ComponentTypes componentColliderType) : Component(parent, componentColliderType)
{
	gMaterial = App->physics->GetDefaultMaterial();
	assert(gMaterial != nullptr);

	App->physics->AddColliderComponent(this);
}

ComponentCollider::~ComponentCollider()
{
	if (parent->cmp_rigidActor != nullptr)
		parent->cmp_rigidActor->UpdateShape(nullptr);
	ClearShape();

	gMaterial = nullptr;

	App->physics->EraseColliderComponent(this);
	parent->cmp_collider = nullptr;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::Checkbox("Is Trigger", &isTrigger))
		SetIsTrigger(isTrigger);
	if (ImGui::Checkbox("Contact Tests", &participateInContactTests))
		SetParticipateInContactTests(participateInContactTests);
	if (ImGui::Checkbox("Scene Queries", &participateInSceneQueries))
		SetParticipateInSceneQueries(participateInSceneQueries);

	// TODO: gMaterial (drag and drop)

	if (componentType != ComponentTypes::PlaneColliderComponent)
	{
		ImGui::Text("Center"); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterX", &center.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterY", &center.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##CenterZ", &center.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetCenter(center);
		ImGui::PopItemWidth();
	}

	if (ImGui::Button("Enclose geometry"))
		EncloseGeometry();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::Update()
{
	if (isTrigger)
	{
		if (triggerEnter && !triggerExit)
			OnTriggerStay(collision);
	}
}

uint ComponentCollider::GetInternalSerializationBytes()
{
	return sizeof(bool) +
		sizeof(bool) +
		sizeof(bool) +
		sizeof(math::float3) +
		sizeof(ColliderTypes);
}

void ComponentCollider::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(cursor, &isTrigger, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &participateInContactTests, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &participateInSceneQueries, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(cursor, &center, bytes);
	cursor += bytes;

	bytes = sizeof(ColliderTypes);
	memcpy(cursor, &colliderType, bytes);
	cursor += bytes;
}

void ComponentCollider::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(&isTrigger, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&participateInContactTests, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&participateInSceneQueries, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(&center, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(ColliderTypes);
	memcpy(&colliderType, cursor, bytes);
	cursor += bytes;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::ClearShape()
{
	if (gShape != nullptr)
		gShape->release();
	gShape = nullptr;
}

void ComponentCollider::SetFiltering(physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // word 0 = own ID
	gShape->setQueryFilterData(filterData);

	filterData.word1 = filterMask; // word 1 = ID mask to filter pairs that trigger a contact callback
	gShape->setSimulationFilterData(filterData);
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::SetIsTrigger(bool isTrigger)
{
	this->isTrigger = isTrigger;
	if (isTrigger && participateInContactTests)
		SetParticipateInContactTests(false); // shapes cannot simultaneously be trigger shapes and simulation shapes
	gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, isTrigger);
}

void ComponentCollider::SetParticipateInContactTests(bool participateInContactTests)
{
	this->participateInContactTests = participateInContactTests;
	if (participateInContactTests && isTrigger)
		SetIsTrigger(false); // shapes cannot simultaneously be trigger shapes and simulation shapes
	gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, participateInContactTests);
}

void ComponentCollider::SetParticipateInSceneQueries(bool participateInSceneQueries)
{
	this->participateInSceneQueries = participateInSceneQueries;
	gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, participateInSceneQueries);
}

void ComponentCollider::SetCenter(const math::float3& center)
{
	assert(center.IsFinite());
	this->center = center;
	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gShape->setLocalPose(relativePose);
}

// ----------------------------------------------------------------------------------------------------

physx::PxShape* ComponentCollider::GetShape() const
{
	return gShape;
}

ColliderTypes ComponentCollider::GetColliderType() const
{
	return colliderType;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnCollisionEnter(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnCollisionEnter with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnCollisionStay(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnCollisionStay with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnCollisionExit(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnCollisionExit with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnTriggerEnter(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnTriggerEnter with '%s'", collision.GetGameObject()->GetName());

	triggerEnter = true;
	triggerExit = false;
	this->collision = collision;
}

void ComponentCollider::OnTriggerStay(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnTriggerStay with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnTriggerExit(Collision& collision)
{
	if (collision.GetGameObject() != nullptr)
		CONSOLE_LOG(LogTypes::Normal, "OnTriggerExit with '%s'", collision.GetGameObject()->GetName());

	triggerExit = true;
	triggerEnter = false;
	this->collision = collision;
}

// ----------------------------------------------------------------------------------------------------

/// Transformed box, sphere, capsule or convex geometry
float ComponentCollider::GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose)
{
	assert(point.IsFinite() && pose.isFinite());
	return physx::PxGeometryQuery::pointDistance(physx::PxVec3(point.x, point.y, point.z), geometry, pose);
}

/// Transformed box, sphere, capsule or convex geometry
float ComponentCollider::GetPointToGeometryObjectDistance(const math::float3& point, const physx::PxGeometry& geometry, const physx::PxTransform& pose, math::float3& closestPoint)
{
	assert(point.IsFinite() && pose.isFinite());
	physx::PxVec3 gClosestPoint;
	float distance = physx::PxGeometryQuery::pointDistance(physx::PxVec3(point.x, point.y, point.z), geometry, pose, &gClosestPoint);
	if (gClosestPoint.isFinite())
		closestPoint = math::float3(gClosestPoint.x, gClosestPoint.y, gClosestPoint.z);
	else
		closestPoint = math::float3::zero;

	return distance;
}

physx::PxBounds3 ComponentCollider::GetGeometryObjectAABB(const physx::PxGeometry& geometry, const physx::PxTransform& pose, float inflation)
{
	physx::PxBounds3 gBounds = physx::PxGeometryQuery::getWorldBounds(geometry, pose, inflation);
	if (!gBounds.isValid() || !gBounds.isFinite())
		gBounds = physx::PxBounds3::empty();

	return gBounds;
}