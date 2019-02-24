#include "ComponentDistanceJoint.h"

#include "Application.h"
#include "GameObject.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentDistanceJoint::ComponentDistanceJoint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::DistanceJoint;
}

ComponentDistanceJoint::ComponentDistanceJoint(const ComponentDistanceJoint& componentDistanceJoint) : ComponentJoint(componentDistanceJoint, ComponentTypes::DistanceJointComponent)
{

}

ComponentDistanceJoint::~ComponentDistanceJoint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentDistanceJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Distance Joint");
	ImGui::Spacing();

	ComponentJoint::OnUniqueEditor();

	if (ImGui::CheckboxFlags("Use min distance", &flags, physx::PxDistanceJointFlag::Enum::eMIN_DISTANCE_ENABLED))
		SetFlag(physx::PxDistanceJointFlag::Enum::eMIN_DISTANCE_ENABLED, flags & physx::PxDistanceJointFlag::Enum::eMIN_DISTANCE_ENABLED);

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Min"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##Max", &limitDistance.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetMinDistance(limitDistance.x);
	ImGui::PopItemWidth();

	if (ImGui::CheckboxFlags("Use max distance", &flags, physx::PxDistanceJointFlag::Enum::eMAX_DISTANCE_ENABLED))
		SetFlag(physx::PxDistanceJointFlag::Enum::eMAX_DISTANCE_ENABLED, flags & physx::PxDistanceJointFlag::Enum::eMAX_DISTANCE_ENABLED);

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##Max", &limitDistance.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetMaxDistance(limitDistance.y);
	ImGui::PopItemWidth();

	if (ImGui::CheckboxFlags("Use spring", &flags, physx::PxDistanceJointFlag::Enum::eSPRING_ENABLED))
		SetFlag(physx::PxDistanceJointFlag::Enum::eSPRING_ENABLED, flags & physx::PxDistanceJointFlag::Enum::eSPRING_ENABLED);

	if (ImGui::TreeNodeEx("Spring", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Stiffness"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Stiffness", &stiffness, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			SetStiffness(stiffness);
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Damping", &damping, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			SetDamping(damping);
		ImGui::PopItemWidth();

		ImGui::TreePop();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentDistanceJoint::Update()
{
	
}

uint ComponentDistanceJoint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentDistanceJoint::OnInternalSave(char*& cursor)
{
}

void ComponentDistanceJoint::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentDistanceJoint::SetMinDistance(float minDistance)
{
	physx::PxDistanceJoint* distanceJoint = (physx::PxDistanceJoint*)gJoint;
	distanceJoint->setMinDistance(minDistance);
}

void ComponentDistanceJoint::SetMaxDistance(float maxDistance)
{
	physx::PxDistanceJoint* distanceJoint = (physx::PxDistanceJoint*)gJoint;
	distanceJoint->setMaxDistance(maxDistance);
}

void ComponentDistanceJoint::SetStiffness(float stiffness)
{
	physx::PxDistanceJoint* distanceJoint = (physx::PxDistanceJoint*)gJoint;
	distanceJoint->setStiffness(stiffness);
}

void ComponentDistanceJoint::SetDamping(float damping)
{
	physx::PxDistanceJoint* distanceJoint = (physx::PxDistanceJoint*)gJoint;
	distanceJoint->setDamping(damping);
}

void ComponentDistanceJoint::SetFlag(physx::PxDistanceJointFlag::Enum flag, bool value)
{
	physx::PxDistanceJoint* distanceJoint = (physx::PxDistanceJoint*)gJoint;
	distanceJoint->setDistanceJointFlag(flag, value);
}