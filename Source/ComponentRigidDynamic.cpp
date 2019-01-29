#include "ComponentRigidDynamic.h"

#include "Application.h"
#include "ModulePhysics.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"

#include "imgui\imgui.h"

ComponentRigidDynamic::ComponentRigidDynamic(GameObject* parent) : ComponentRigidActor(parent, ComponentTypes::RigidDynamicComponent)
{
	physx::PxShape* gShape = nullptr;
	if (parent->boundingBox.IsFinite())
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(parent->boundingBox.HalfSize().x, parent->boundingBox.HalfSize().y, parent->boundingBox.HalfSize().z), *App->physics->GetDefaultMaterial());
	else
		gShape = App->physics->CreateShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), *App->physics->GetDefaultMaterial());

	gActor = App->physics->CreateRigidDynamic(physx::PxTransform(physx::PxIDENTITY()), *gShape, 10.0f, isKinematic);
	if (gActor == nullptr)
		return;

	SetMass(mass);
	SetLinearDamping(linearDamping);
	SetAngularDamping(angularDamping);
	SetUseGravity(useGravity);

	// ----------

	if (parent->collider != nullptr)
		UpdateShape();
	UpdateTransform();
}

ComponentRigidDynamic::~ComponentRigidDynamic() {}

void ComponentRigidDynamic::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Rigid Dynamic");
	ImGui::Spacing();

	ComponentRigidActor::OnUniqueEditor();

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	// TODO: cap min to 0

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Mass"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##Mass", ImGuiDataType_Float, (void*)&mass, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		SetMass(mass);

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Linear damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##LinearDamping", ImGuiDataType_Float, (void*)&linearDamping, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		SetLinearDamping(linearDamping);

	// -----

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Angular damping"); ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##AngularDamping", ImGuiDataType_Float, (void*)&angularDamping, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f))
		SetAngularDamping(angularDamping);

	// -----

	if (ImGui::Checkbox("Is Kinematic", &isKinematic))
		SetIsKinematic(isKinematic);

	// --------------------------------------------------

	ImGui::Spacing();

	const ImVec4 disabledTextColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

	ImGui::Text("Linear velocity");

	physx::PxVec3 gCurrentLinearVelocity = gActor->is<physx::PxRigidDynamic>()->getLinearVelocity();
	math::float3 currentLinearVelocity = math::float3(gCurrentLinearVelocity.x, gCurrentLinearVelocity.y, gCurrentLinearVelocity.z);
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.x); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.y); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", currentLinearVelocity.z);

	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##LinearVelocityX", ImGuiDataType_Float, (void*)&linearVelocity.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##LinearVelocityY", ImGuiDataType_Float, (void*)&linearVelocity.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##LinearVelocityZ", ImGuiDataType_Float, (void*)&linearVelocity.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	
	ImGui::PushID("LinearVelocityButton");
	if (ImGui::SmallButton("Apply"))
		SetLinearVelocity(linearVelocity);
	ImGui::PopID();

	// -----

	ImGui::Text("Angular velocity");

	physx::PxVec3 gCurrentAngularVelocity = gActor->is<physx::PxRigidDynamic>()->getAngularVelocity();
	math::float3 currentAngularVelocity = math::float3(gCurrentAngularVelocity.x, gCurrentAngularVelocity.y, gCurrentAngularVelocity.z);
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.x); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.y); ImGui::SameLine();
	ImGui::TextColored(disabledTextColor, "%.2f", gCurrentAngularVelocity.z);

	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##AngularVelocityX", ImGuiDataType_Float, (void*)&angularVelocity.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##AngularVelocityY", ImGuiDataType_Float, (void*)&angularVelocity.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##AngularVelocityZ", ImGuiDataType_Float, (void*)&angularVelocity.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	
	ImGui::PushID("AngularVelocityButton");
	if (ImGui::SmallButton("Apply"))
		SetAngularVelocity(angularVelocity);
	ImGui::PopID();

	// -----

	ImGui::Text("Force"); 
	
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##ForceX", ImGuiDataType_Float, (void*)&force.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##ForceY", ImGuiDataType_Float, (void*)&force.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##ForceZ", ImGuiDataType_Float, (void*)&force.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);

	ImGui::PushID("AddForceButton");
	if (ImGui::SmallButton("Apply"))
		AddForce(force);
	ImGui::PopID();

	ImGui::SameLine();

	ImGui::PushID("ClearForceButton");
	if (ImGui::SmallButton("Clear"))
		ClearForce();
	ImGui::PopID();

	// -----

	ImGui::Text("Torque"); 
	
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##TorqueX", ImGuiDataType_Float, (void*)&torque.x, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##TorqueY", ImGuiDataType_Float, (void*)&torque.y, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##TorqueZ", ImGuiDataType_Float, (void*)&torque.z, 0.01f, &f64_lo_a, &f64_hi_a, "%.2f", 1.0f);

	ImGui::PushID("AddTorqueButton");
	if (ImGui::SmallButton("Apply"))
		AddTorque(torque);
	ImGui::PopID();

	ImGui::SameLine();

	ImGui::PushID("ClearTorqueButton");
	if (ImGui::SmallButton("Clear"))
		ClearTorque();
	ImGui::PopID();
#endif
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::SetMass(float mass) const
{
	gActor->is<physx::PxRigidDynamic>()->setMass(mass);
}

void ComponentRigidDynamic::SetLinearDamping(float linearDamping) const
{
	gActor->is<physx::PxRigidDynamic>()->setLinearDamping(linearDamping);
}

void ComponentRigidDynamic::SetAngularDamping(float angularDamping) const
{
	gActor->is<physx::PxRigidDynamic>()->setAngularDamping(angularDamping);
}

void ComponentRigidDynamic::SetIsKinematic(bool isKinematic) const
{
	gActor->is<physx::PxRigidBody>()->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, isKinematic);
}

// ----------------------------------------------------------------------------------------------------

void ComponentRigidDynamic::SetLinearVelocity(math::float3 linearVelocity) const
{
	gActor->is<physx::PxRigidDynamic>()->setLinearVelocity(physx::PxVec3(linearVelocity.x, linearVelocity.y, linearVelocity.z));
}

void ComponentRigidDynamic::SetAngularVelocity(math::float3 angularVelocity) const
{
	gActor->is<physx::PxRigidDynamic>()->setAngularVelocity(physx::PxVec3(angularVelocity.x, angularVelocity.y, angularVelocity.z));
}

void ComponentRigidDynamic::AddForce(math::float3 force) const
{
	gActor->is<physx::PxRigidDynamic>()->addForce(physx::PxVec3(force.x, force.y, force.z));
}

void ComponentRigidDynamic::ClearForce() const
{
	gActor->is<physx::PxRigidDynamic>()->clearForce();
}

void ComponentRigidDynamic::AddTorque(math::float3 torque) const
{
	gActor->is<physx::PxRigidDynamic>()->addTorque(physx::PxVec3(torque.x, torque.y, torque.z));
}

void ComponentRigidDynamic::ClearTorque() const
{
	gActor->is<physx::PxRigidDynamic>()->clearTorque();
}