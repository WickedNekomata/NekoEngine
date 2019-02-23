#include "ComponentRevoluteJoint.h"

#include "Application.h"
#include "GameObject.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentRevoluteJoint::ComponentRevoluteJoint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::RevoluteJoint;
}

ComponentRevoluteJoint::ComponentRevoluteJoint(const ComponentRevoluteJoint& componentRevoluteJoint) : ComponentJoint(componentRevoluteJoint, ComponentTypes::RevoluteJointComponent)
{

}

ComponentRevoluteJoint::~ComponentRevoluteJoint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentRevoluteJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Revolute Joint");
	ImGui::Spacing();

	ComponentJoint::OnUniqueEditor();

	if (ImGui::CheckboxFlags("Use limits", &flags, physx::PxRevoluteJointFlag::Enum::eLIMIT_ENABLED))
		SetFlag(physx::PxRevoluteJointFlag::Enum::eLIMIT_ENABLED, flags & physx::PxRevoluteJointFlag::Enum::eLIMIT_ENABLED);

	if (ImGui::TreeNodeEx("Limits", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::AlignTextToFramePadding();
			ImGui::Text("Min"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("##Min", &limitAngle.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			{
				if (useSpring)
					SetSoftLimit(limitAngle, stiffness, damping);
				else
					SetHardLimit(limitAngle, contactDistance);
			}
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Max", &limitAngle.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		{
			if (useSpring)
				SetSoftLimit(limitAngle, stiffness, damping);
			else
				SetHardLimit(limitAngle, contactDistance);
		}
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Contact distance"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##ContactDistance", &contactDistance, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		{
			if (useSpring)
				SetSoftLimit(limitAngle, stiffness, damping);
			else
				SetHardLimit(limitAngle, contactDistance);
		}
		ImGui::PopItemWidth();

		ImGui::Checkbox("Use spring", &useSpring);
		if (ImGui::TreeNodeEx("Spring", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Stiffness"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("##Stiffness", &stiffness, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			{
				if (useSpring)
					SetSoftLimit(limitAngle, stiffness, damping);
				else
					SetHardLimit(limitAngle, contactDistance);
			}
			ImGui::PopItemWidth();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("##Damping", &damping, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			{
				if (useSpring)
					SetSoftLimit(limitAngle, stiffness, damping);
				else
					SetHardLimit(limitAngle, contactDistance);
			}
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if (ImGui::CheckboxFlags("Use motor", &flags, physx::PxRevoluteJointFlag::Enum::eDRIVE_ENABLED))
		SetFlag(physx::PxRevoluteJointFlag::Enum::eDRIVE_ENABLED, flags & physx::PxRevoluteJointFlag::Enum::eDRIVE_ENABLED);

	if (ImGui::TreeNodeEx("Motor", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Target velocity"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##TargetVelocity", &targetVelocity, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			SetTargetVelocity(targetVelocity);
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Force"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Force", &force, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			SetForce(force);
		ImGui::PopItemWidth();

		if (ImGui::CheckboxFlags("Free Spin", &flags, physx::PxRevoluteJointFlag::Enum::eDRIVE_FREESPIN))
			SetFlag(physx::PxRevoluteJointFlag::Enum::eDRIVE_FREESPIN, flags & physx::PxRevoluteJointFlag::Enum::eDRIVE_FREESPIN);

		ImGui::TreePop();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRevoluteJoint::Update()
{
	
}

uint ComponentRevoluteJoint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentRevoluteJoint::OnInternalSave(char*& cursor)
{
}

void ComponentRevoluteJoint::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentRevoluteJoint::SetHardLimit(math::float2& limitAngle, float contactDistance)
{
	physx::PxRevoluteJoint* revoluteJoint = (physx::PxRevoluteJoint*)gJoint;
	revoluteJoint->setLimit(physx::PxJointAngularLimitPair(limitAngle.x, limitAngle.y, contactDistance)); // angular hard limit pair
}

void ComponentRevoluteJoint::SetSoftLimit(math::float2& limitAngle, float stiffness, float damping)
{
	physx::PxRevoluteJoint* revoluteJoint = (physx::PxRevoluteJoint*)gJoint;
	revoluteJoint->setLimit(physx::PxJointAngularLimitPair(limitAngle.x, limitAngle.y, physx::PxSpring(stiffness, damping))); // angular soft limit pair
}

void ComponentRevoluteJoint::SetTargetVelocity(float targetVelocity)
{
	physx::PxRevoluteJoint* revoluteJoint = (physx::PxRevoluteJoint*)gJoint;
	revoluteJoint->setDriveVelocity(targetVelocity);
}

void ComponentRevoluteJoint::SetForce(float force)
{
	physx::PxRevoluteJoint* revoluteJoint = (physx::PxRevoluteJoint*)gJoint;
	revoluteJoint->setDriveForceLimit(force);
}

void ComponentRevoluteJoint::SetFlag(physx::PxRevoluteJointFlag::Enum flag, bool value)
{
	physx::PxRevoluteJoint* revoluteJoint = (physx::PxRevoluteJoint*)gJoint;
	revoluteJoint->setRevoluteJointFlag(flag, value);
}