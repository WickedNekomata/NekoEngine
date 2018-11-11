#include "ComponentTransform.h"
#include "GameObject.h"

#include "ComponentCamera.h"
#include "Application.h"
#include "ModuleTimeManager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <list>

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentType::Transform_Component) {
}

ComponentTransform::ComponentTransform(const ComponentTransform& componentTransform) : Component(componentTransform.parent, ComponentType::Transform_Component)
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
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool seenLastFrame = parent->GetSeenLastFrame();
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

	/*
	if (lastPosition.x != position.x || lastPosition.y != position.y || lastPosition.z != position.z
		|| lastRotation.x != rotation.x || lastRotation.y != rotation.y || lastRotation.z != rotation.z || lastRotation.w != rotation.w
		|| lastScale.x != scale.x || lastScale.y != scale.y || lastScale.z != scale.z)
		parent->RecursiveRecalculateBoundingBoxes();
		*/
	if (parent->camera != nullptr)
		parent->camera->UpdateTransform();
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
	globalMatrix = globalMatrix * localMatrix;

	return globalMatrix;
}

void ComponentTransform::SetMatrixFromGlobal(math::float4x4& globalMatrix)
{
	if (parent->GetParent() == App->scene->root) {
		globalMatrix.Decompose(position, rotation, scale);
		return;
	}

	math::float4x4 newMatrix = parent->GetParent()->transform->GetGlobalMatrix();
	newMatrix = newMatrix.Inverted();
	newMatrix = newMatrix * globalMatrix;

	newMatrix.Decompose(position, rotation, scale);
}

void ComponentTransform::SetMatrixFromLocal(math::float4x4& localMatrix)
{
	localMatrix.Decompose(position, rotation, scale);
}

void ComponentTransform::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "PosX", position.x);
	json_object_set_number(file, "PosY", position.y);
	json_object_set_number(file, "PosZ", position.z);
	json_object_set_number(file, "RotX", rotation.x);
	json_object_set_number(file, "RotY", rotation.y);
	json_object_set_number(file, "RotZ", rotation.z);
	json_object_set_number(file, "RotW", rotation.w);
	json_object_set_number(file, "ScaleX", scale.x);
	json_object_set_number(file, "ScaleY", scale.y);
	json_object_set_number(file, "ScaleZ", scale.z);
}

void ComponentTransform::OnLoad(JSON_Object* file)
{
	position.x = json_object_get_number(file, "PosX");
	position.y = json_object_get_number(file, "PosY");
	position.z = json_object_get_number(file, "PosZ");
	rotation.x = json_object_get_number(file, "RotX");
	rotation.y = json_object_get_number(file, "RotY");
	rotation.z = json_object_get_number(file, "RotZ");
	rotation.w = json_object_get_number(file, "RotW");
	scale.x = json_object_get_number(file, "ScaleX");
	scale.y = json_object_get_number(file, "ScaleY");
	scale.z = json_object_get_number(file, "ScaleZ");
}