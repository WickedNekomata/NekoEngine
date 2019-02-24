#include "ComponentFixedJoint.h"

#include "Application.h"
#include "GameObject.h"

#include <assert.h>

#include "imgui\imgui.h"

ComponentFixedJoint::ComponentFixedJoint(GameObject* parent, ComponentTypes componentJointType) : ComponentJoint(parent, componentJointType)
{
	assert(parent->cmp_rigidActor != nullptr);
	jointType = JointTypes::FixedJoint;
}

ComponentFixedJoint::ComponentFixedJoint(const ComponentFixedJoint& componentFixedJoint) : ComponentJoint(componentFixedJoint, ComponentTypes::FixedJointComponent)
{

}

ComponentFixedJoint::~ComponentFixedJoint()
{

}

// ----------------------------------------------------------------------------------------------------

void ComponentFixedJoint::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Fixed Joint");
	ImGui::Spacing();

	ComponentJoint::OnUniqueEditor();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentFixedJoint::Update()
{

}

uint ComponentFixedJoint::GetInternalSerializationBytes()
{
	return uint();
}

void ComponentFixedJoint::OnInternalSave(char *& cursor)
{
}

void ComponentFixedJoint::OnInternalLoad(char *& cursor)
{
}

// ----------------------------------------------------------------------------------------------------