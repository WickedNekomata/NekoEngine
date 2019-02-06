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
}

ComponentCollider::~ComponentCollider() 
{
	if (parent->rigidActor != nullptr)
		parent->rigidActor->UpdateShape(nullptr);
	ClearShape();

	gMaterial = nullptr;
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
	filterData.word1 = filterMask; // word 1 = ID mask to filter pairs that trigger a contact callback
	
	gShape->setSimulationFilterData(filterData);
	gShape->setQueryFilterData(filterData);
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::SetIsTrigger(bool isTrigger)
{
	this->isTrigger = isTrigger;
	gShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger); // shapes cannot simultaneously be trigger shapes and simulation shapes
	gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, isTrigger);
}

void ComponentCollider::SetParticipateInContactTests(bool participateInContactTests)
{
	this->participateInContactTests = participateInContactTests;
	gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, participateInContactTests);
}

void ComponentCollider::SetParticipateInSceneQueries(bool participateInSceneQueries)
{
	this->participateInSceneQueries = participateInSceneQueries;
	gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, participateInSceneQueries);
}

void ComponentCollider::SetCenter(math::float3& center)
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

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnCollisionEnter(Collision& collision)
{
	CONSOLE_LOG(LogTypes::Normal, "OnCollisionEnter with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnCollisionStay(Collision& collision)
{
	CONSOLE_LOG(LogTypes::Normal, "OnCollisionStay with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnCollisionExit(Collision& collision)
{
	CONSOLE_LOG(LogTypes::Normal, "OnCollisionExit with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnTriggerEnter(Collision& collision)
{
	CONSOLE_LOG(LogTypes::Normal, "OnTriggerEnter with '%s'", collision.GetGameObject()->GetName());

	triggerEnter = true;
	triggerExit = false;
	this->collision = collision;
}

void ComponentCollider::OnTriggerStay(Collision& collision)
{
	CONSOLE_LOG(LogTypes::Normal, "OnTriggerStay with '%s'", collision.GetGameObject()->GetName());
}

void ComponentCollider::OnTriggerExit(Collision& collision)
{
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