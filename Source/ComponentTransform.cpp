#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleCameraEditor.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentRigidActor.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include <list>

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentTypes::TransformComponent) {}

ComponentTransform::ComponentTransform(const ComponentTransform& componentTransform) : Component(componentTransform.parent, ComponentTypes::TransformComponent)
{
	position = componentTransform.position;
	rotation = componentTransform.rotation;
	scale = componentTransform.scale;
}

ComponentTransform::~ComponentTransform()
{
	parent->transform = nullptr;
}

void ComponentTransform::Update() {}

// Redefined cause there is no way that a transform component could be erased or moved.
void ComponentTransform::OnEditor()
{
	OnUniqueEditor();
}

void ComponentTransform::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool seenLastFrame = parent->seenLastFrame;
	ImGui::Checkbox("Seen last frame", &seenLastFrame);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

	math::float3 lastPosition = position;
	math::Quat lastRotation = rotation;
	math::float3 lastScale = scale;

	if (ImGui::Button("Reset"))
	{
		position = math::float3::zero;
		rotation = math::Quat::identity;
		scale = math::float3::one;
	}

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Position");
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosX", ImGuiDataType_Float, (void*)&position.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosY", ImGuiDataType_Float, (void*)&position.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosZ", ImGuiDataType_Float, (void*)&position.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	ImGui::Text("Rotation");
	math::float3 axis;
	float angle;
	rotation.ToAxisAngle(axis, angle);
	axis *= angle;
	axis *= RADTODEG;
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##AxisAngleX", ImGuiDataType_Float, (void*)&axis.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##AxisAngleY", ImGuiDataType_Float, (void*)&axis.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##AxisAngleZ", ImGuiDataType_Float, (void*)&axis.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);
	axis *= DEGTORAD;
	rotation.SetFromAxisAngle(axis.Normalized(), axis.Length());

	ImGui::Text("Scale");
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleX", ImGuiDataType_Float, (void*)&scale.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleY", ImGuiDataType_Float, (void*)&scale.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleZ", ImGuiDataType_Float, (void*)&scale.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	if (!position.Equals(lastPosition) || !rotation.Equals(lastRotation) || !scale.Equals(lastScale))
	{
		// Transform updated: if the game object has a rigid body, update its transform
		if (parent->cmp_rigidActor != nullptr)
		{
			math::float4x4 globalMatrix = GetGlobalMatrix();
			parent->cmp_rigidActor->UpdateTransform(globalMatrix);
		}

		// Transform updated: if the game object has a camera, update its frustum
		if (parent->cmp_camera != nullptr)
			parent->cmp_camera->UpdateTransform();

#ifndef GAMEMODE
		// Transform updated: if the game object is selected, update the camera reference
		if (parent == App->scene->selectedObject.Get())
			App->camera->SetReference(position);
#endif

		// Transform updated: recalculate bounding boxes
		System_Event newEvent;
		newEvent.goEvent.gameObject = parent;
		newEvent.type = System_Event_Type::RecalculateBBoxes;
		App->PushSystemEvent(newEvent);

		if (parent->IsStatic())
		{
			// Bounding box changed: recreate quadtree
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
		}
	}
#endif // !GAMEMODE
}

math::float4x4& ComponentTransform::GetMatrix() const
{
	math::float4x4 matrix = math::float4x4::FromTRS(position, rotation, scale);
	return matrix;
}

math::float4x4& ComponentTransform::GetGlobalMatrix() const
{
	std::list<GameObject*> aux_list;

	GameObject* globalParent = this->GetParent()->GetParent();

	while (globalParent->GetParent() != nullptr)
	{
		aux_list.push_back(globalParent);
		globalParent = globalParent->GetParent();
	}
	
	math::float4x4 globalMatrix = math::float4x4::identity;

	for (std::list<GameObject*>::const_reverse_iterator it = aux_list.rbegin(); it != aux_list.rend(); it++)
	{
		math::float4x4 parentMatrix = (*it)->transform->GetMatrix();
		globalMatrix = globalMatrix * parentMatrix;
	}

	math::float4x4 localMatrix = GetMatrix();

	return globalMatrix * localMatrix;
}

void ComponentTransform::SetMatrixFromGlobal(math::float4x4& globalMatrix)
{
	if (parent->GetParent() == App->scene->root) 
		globalMatrix.Decompose(position, rotation, scale);
	else
	{
		math::float4x4 newMatrix = parent->GetParent()->transform->GetGlobalMatrix();
		newMatrix = newMatrix.Inverted();
		newMatrix = newMatrix * globalMatrix;

		newMatrix.Decompose(position, rotation, scale);
	}

	// Transform updated: if the game object has a rigid body, update its transform
	if (parent->cmp_rigidActor != nullptr)
	{
		math::float4x4 globalMatrix = GetGlobalMatrix();
		parent->cmp_rigidActor->UpdateTransform(globalMatrix);
	}

	// Transform updated: if the game object has a camera, update its frustum
	if (parent->cmp_camera != nullptr)
		parent->cmp_camera->UpdateTransform();

#ifndef GAMEMODE
	// Transform updated: if the game object is selected, update the camera reference
	if (parent == App->scene->selectedObject.Get())
		App->camera->SetReference(position);
#endif

	// Transform updated: recalculate bounding boxes
	System_Event newEvent;
	newEvent.goEvent.gameObject = parent;
	newEvent.type = System_Event_Type::RecalculateBBoxes;
	App->PushSystemEvent(newEvent);

	if (parent->IsStatic())
	{
		// Bounding box changed: recreate quadtree
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
	}
}

uint ComponentTransform::GetInternalSerializationBytes()
{
	// position + scale + rotation
	return sizeof(math::float3) * 2 + sizeof(math::Quat);
}

void ComponentTransform::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(math::float3);
	memcpy(cursor, &position, bytes);
	cursor += bytes;

	bytes = sizeof(math::Quat);
	memcpy(cursor, &rotation, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(cursor, &scale, bytes);
	cursor += bytes;
}

void ComponentTransform::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(math::float3);
	memcpy(&position, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::Quat);
	memcpy(&rotation, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(math::float3);
	memcpy(&scale, cursor, bytes);
	cursor += bytes;
}