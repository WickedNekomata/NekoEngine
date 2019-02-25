#include "ComponentJoint.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidDynamic.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentJoint::ComponentJoint(GameObject* parent, ComponentTypes componentJointType) : Component(parent, componentJointType)
{
	App->physics->AddJointComponent(this);
}

ComponentJoint::ComponentJoint(const ComponentJoint& componentJoint, ComponentTypes componentJointType) : Component(componentJoint.parent, componentJointType)
{

}

ComponentJoint::~ComponentJoint()
{
	App->physics->EraseJointComponent(this);
	//parent->cmp_joint = nullptr;
}

// ----------------------------------------------------------------------------------------------------

void ComponentJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	//*****Debug*****
	const ImVec4 disabledTextColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

	if (parent->cmp_rigidActor->GetRigidActorType() == RigidActorTypes::RigidDynamic
		&& !((ComponentRigidDynamic*)parent->cmp_rigidActor)->IsSleeping())
	{
		physx::PxVec3 force, torque;
		gJoint->getConstraint()->getForce(force, torque);

		ImGui::Text("Force");
		ImGui::TextColored(disabledTextColor, "%.2f", force.x); ImGui::SameLine();
		ImGui::TextColored(disabledTextColor, "%.2f", force.y); ImGui::SameLine();
		ImGui::TextColored(disabledTextColor, "%.2f", force.z);

		ImGui::Text("Torque");
		ImGui::TextColored(disabledTextColor, "%.2f", torque.x); ImGui::SameLine();
		ImGui::TextColored(disabledTextColor, "%.2f", torque.y); ImGui::SameLine();
		ImGui::TextColored(disabledTextColor, "%.2f", torque.z);
	}
	//_*****Debug*****

	ImGui::Spacing();

	// TODO: connected body drag and drop

	ImGui::Text("Center"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterX", &center.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR0, center);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterY", &center.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR0, center);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##CenterZ", &center.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR0, center);
	ImGui::PopItemWidth();

	ImGui::Text("Connected center"); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##ConnectedCenterX", &connectedCenter.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR1, connectedCenter);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##ConnectedCenterY", &connectedCenter.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR1, connectedCenter);
	ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##ConnectedCenterZ", &connectedCenter.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
		SetCenter(physx::PxJointActorIndex::Enum::eACTOR1, connectedCenter);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Break force"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##BreakForce", &breakForce, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetBreakForce(breakForce);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Break torque"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##BreakTorque", &breakTorque, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetBreakTorque(breakTorque);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Mass scale"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##MassScale", &massScale, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetMassScale(physx::PxJointActorIndex::Enum::eACTOR0, massScale);
	ImGui::PopItemWidth();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Connected mass scale"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragFloat("##ConnectedMassScale", &connectedMassScale, 0.01f, 0.0f, FLT_MAX, "%.2f", 1.0f))
		SetMassScale(physx::PxJointActorIndex::Enum::eACTOR1, connectedMassScale);
	ImGui::PopItemWidth();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentJoint::Update()
{
}

// ----------------------------------------------------------------------------------------------------

void ComponentJoint::RecalculateJoint()
{
	// TODO: clear joint (if any)
	assert(connectedBody != nullptr);
	physx::PxRigidActor* actor0 = parent->cmp_rigidActor->GetActor();
	physx::PxRigidActor* actor1 = connectedBody->GetActor();
	gJoint = App->physics->CreateJoint(jointType, actor0, actor0->getGlobalPose(), actor1, actor1->getGlobalPose());
	assert(gJoint != nullptr);
}

// ----------------------------------------------------------------------------------------------------

void ComponentJoint::SetCenter(physx::PxJointActorIndex::Enum actor, math::float3& center)
{
	assert(center.IsFinite());
	this->center = center;
	physx::PxTransform relativePose(physx::PxVec3(center.x, center.y, center.z));
	gJoint->setLocalPose(actor, relativePose);
}

void ComponentJoint::SetBreakForce(float breakForce)
{
	this->breakForce = breakForce;
	gJoint->setBreakForce(breakForce, breakTorque);
}

void ComponentJoint::SetBreakTorque(float breakTorque)
{
	this->breakTorque = breakTorque;
	gJoint->setBreakForce(breakForce, breakTorque);
}

void ComponentJoint::SetMassScale(physx::PxJointActorIndex::Enum actor, float massScale)
{
	switch (actor)
	{
	case physx::PxJointActorIndex::Enum::eACTOR0:
		gJoint->setInvMassScale0(massScale);
		break;
	case physx::PxJointActorIndex::Enum::eACTOR1:
		gJoint->setInvMassScale1(massScale);
		break;
	}
}

physx::PxJoint* ComponentJoint::GetJoint() const
{
	return gJoint;
}

JointTypes ComponentJoint::GetJointType() const
{
	return jointType;
}

bool ComponentJoint::IsBroken() const
{
	return gJoint->getConstraintFlags() & physx::PxConstraintFlag::eBROKEN;
}