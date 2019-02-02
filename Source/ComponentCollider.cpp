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
	if (ImGui::Checkbox("Is Trigger", &isTrigger))
		SetIsTrigger(isTrigger);	
	if (ImGui::Checkbox("Contact Tests", &participateInContactTests))
		ParticipateInContactTests(participateInContactTests);
	if (ImGui::Checkbox("Scene Queries", &participateInSceneQueries))
		ParticipateInSceneQueries(participateInSceneQueries);

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

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::Update()
{
	if (isTrigger)
	{
		if (triggerEnter && !triggerExit)
			OnTriggerStay();
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
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::SetIsTrigger(bool isTrigger)
{
	this->isTrigger = isTrigger;
	gShape->setFlag(physx::PxShapeFlag::Enum::eTRIGGER_SHAPE, isTrigger);
}

void ComponentCollider::ParticipateInContactTests(bool participateInContactTests)
{
	this->participateInContactTests = participateInContactTests;
	gShape->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, participateInContactTests);
}

void ComponentCollider::ParticipateInSceneQueries(bool participateInSceneQueries)
{
	this->participateInSceneQueries = participateInSceneQueries;
	gShape->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, participateInSceneQueries);
}

// ----------------------------------------------------------------------------------------------------

physx::PxShape* ComponentCollider::GetShape() const
{
	return gShape;
}

// ----------------------------------------------------------------------------------------------------

void ComponentCollider::OnCollisionEnter()
{
	CONSOLE_LOG("OnCollisionEnter");
}

void ComponentCollider::OnCollisionStay()
{
	CONSOLE_LOG("OnCollisionStay");
}

void ComponentCollider::OnCollisionExit()
{
	CONSOLE_LOG("OnCollisionExit");
}

void ComponentCollider::OnTriggerEnter()
{
	CONSOLE_LOG("OnTriggerEnter");
	triggerEnter = true;
	triggerExit = false;
}

void ComponentCollider::OnTriggerStay()
{
	CONSOLE_LOG("OnTriggerStay");
}

void ComponentCollider::OnTriggerExit()
{
	CONSOLE_LOG("OnTriggerExit");
	triggerExit = true;
	triggerEnter = false;
}