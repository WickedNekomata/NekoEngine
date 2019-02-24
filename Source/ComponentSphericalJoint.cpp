#include "ComponentSphericalJoint.h"

#include "Application.h"
#include "GameObject.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentSphericalJoint::ComponentSphericalJoint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::SphericalJoint;
}

ComponentSphericalJoint::ComponentSphericalJoint(const ComponentSphericalJoint& componentSphericalJoint) : ComponentJoint(componentSphericalJoint, ComponentTypes::SphericalJointComponent)
{

}

ComponentSphericalJoint::~ComponentSphericalJoint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentSphericalJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Spherical Joint");
	ImGui::Spacing();

	ComponentJoint::OnUniqueEditor();

	if (ImGui::CheckboxFlags("Use limits", &flags, physx::PxSphericalJointFlag::Enum::eLIMIT_ENABLED))
		SetFlag(physx::PxSphericalJointFlag::Enum::eLIMIT_ENABLED, flags & physx::PxSphericalJointFlag::Enum::eLIMIT_ENABLED);

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
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphericalJoint::Update()
{

}

uint ComponentSphericalJoint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentSphericalJoint::OnInternalSave(char*& cursor)
{
}

void ComponentSphericalJoint::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentSphericalJoint::SetHardLimit(math::float2& limitAngle, float contactDistance)
{
	physx::PxSphericalJoint* sphericalJoint = (physx::PxSphericalJoint*)gJoint;
	sphericalJoint->setLimitCone(physx::PxJointLimitCone(limitAngle.x, limitAngle.y, contactDistance)); // cone hard limit
}

void ComponentSphericalJoint::SetSoftLimit(math::float2& limitAngle, float stiffness, float damping)
{
	physx::PxSphericalJoint* sphericalJoint = (physx::PxSphericalJoint*)gJoint;
	sphericalJoint->setLimitCone(physx::PxJointLimitCone(limitAngle.x, limitAngle.y, physx::PxSpring(stiffness, damping))); // cone soft limit
}

void ComponentSphericalJoint::SetFlag(physx::PxSphericalJointFlag::Enum flag, bool value)
{
	physx::PxSphericalJoint* sphericalJoint = (physx::PxSphericalJoint*)gJoint;
	sphericalJoint->setSphericalJointFlag(flag, value);
}