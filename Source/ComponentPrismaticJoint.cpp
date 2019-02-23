#include "ComponentPrismaticJoint.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentPrismaticJoint::ComponentPrismaticJoint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::PrismaticJoint;
}

ComponentPrismaticJoint::ComponentPrismaticJoint(const ComponentPrismaticJoint& componentPrismaticJoint) : ComponentJoint(componentPrismaticJoint, ComponentTypes::PrismaticJointComponent)
{

}

ComponentPrismaticJoint::~ComponentPrismaticJoint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentPrismaticJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Prismatic Joint");
	ImGui::Spacing();

	ComponentJoint::OnUniqueEditor();

	if (ImGui::CheckboxFlags("Use limits", &flags, physx::PxPrismaticJointFlag::Enum::eLIMIT_ENABLED))
		SetFlag(physx::PxPrismaticJointFlag::Enum::eLIMIT_ENABLED, flags & physx::PxPrismaticJointFlag::Enum::eLIMIT_ENABLED);

	if (ImGui::TreeNodeEx("Limits", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Min"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Min", &limitDistance.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		{
			if (useSpring)
				SetSoftLimit(limitDistance, stiffness, damping);
			else
				SetHardLimit(limitDistance, contactDistance);
		}
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##Max", &limitDistance.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		{
			if (useSpring)
				SetSoftLimit(limitDistance, stiffness, damping);
			else
				SetHardLimit(limitDistance, contactDistance);
		}
		ImGui::PopItemWidth();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Contact distance"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##ContactDistance", &contactDistance, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		{
			if (useSpring)
				SetSoftLimit(limitDistance, stiffness, damping);
			else
				SetHardLimit(limitDistance, contactDistance);
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
					SetSoftLimit(limitDistance, stiffness, damping);
				else
					SetHardLimit(limitDistance, contactDistance);
			}
			ImGui::PopItemWidth();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("##Damping", &damping, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
			{
				if (useSpring)
					SetSoftLimit(limitDistance, stiffness, damping);
				else
					SetHardLimit(limitDistance, contactDistance);
			}
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentPrismaticJoint::Update()
{
	
}

uint ComponentPrismaticJoint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentPrismaticJoint::OnInternalSave(char*& cursor)
{
}

void ComponentPrismaticJoint::OnInternalLoad(char*& cursor)
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentPrismaticJoint::SetHardLimit(math::float2& limitAngle, float contactDistance)
{
	physx::PxPrismaticJoint* prismaticJoint = (physx::PxPrismaticJoint*)gJoint;
	prismaticJoint->setLimit(physx::PxJointLinearLimitPair(App->physics->GetTolerancesScale(), limitDistance.x, limitDistance.y, contactDistance)); // linear hard limit pair
}

void ComponentPrismaticJoint::SetSoftLimit(math::float2& limitAngle, float stiffness, float damping)
{
	physx::PxPrismaticJoint* prismaticJoint = (physx::PxPrismaticJoint*)gJoint;
	prismaticJoint->setLimit(physx::PxJointLinearLimitPair(limitDistance.x, limitDistance.y, physx::PxSpring(stiffness, damping))); // linear soft limit pair
}

void ComponentPrismaticJoint::SetFlag(physx::PxPrismaticJointFlag::Enum flag, bool value)
{
	physx::PxPrismaticJoint* prismaticJoint = (physx::PxPrismaticJoint*)gJoint;
	prismaticJoint->setPrismaticJointFlag(flag, value);
}