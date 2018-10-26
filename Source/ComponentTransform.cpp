#include "ComponentTransform.h"
#include "GameObject.h"

#include "imgui/imgui.h"

#include <list>

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, ComponentType::Transform_Component)
{
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::Update() const
{
}

void ComponentTransform::OnUniqueEditor()
{
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	ImGui::Text("Position");

	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosX", ImGuiDataType_Float, (void*)&position.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosY", ImGuiDataType_Float, (void*)&position.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##PosZ", ImGuiDataType_Float, (void*)&position.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	ImGui::Text("Rotation");
	math::float3 euler = rotation.ToEulerXYZ();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##EulerX", ImGuiDataType_Float, (void*)&euler.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##EulerY", ImGuiDataType_Float, (void*)&euler.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##EulerZ", ImGuiDataType_Float, (void*)&euler.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	rotation = math::Quat::FromEulerXYZ(euler.x, euler.y, euler.z);

	ImGui::Text("Scale");

	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleX", ImGuiDataType_Float, (void*)&scale.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleY", ImGuiDataType_Float, (void*)&scale.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleZ", ImGuiDataType_Float, (void*)&scale.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);
}

math::float4x4 ComponentTransform::GetMatrix()
{
	return math::float4x4::FromTRS(position, rotation, scale);
}

math::float4x4 ComponentTransform::GetGlobalMatrix()
{
	std::list<GameObject*> aux_list;

	GameObject* globalParent = this->GetParent()->GetParent();

	while (globalParent->GetParent() != nullptr)
	{
		aux_list.push_back(globalParent);
		globalParent = globalParent->GetParent();
	}
	
	math::float4x4 matrix = math::float4x4::identity;

	for (std::list<GameObject*>::const_reverse_iterator it = aux_list.rbegin(); it != aux_list.rend(); it++)
	{
		if (it == aux_list.rbegin()) 
		{
			matrix = (*it)->transform->GetMatrix();
			continue;
		}

		matrix = matrix * (*it)->transform->GetMatrix();
	}

	math::float4x4 other = GetMatrix();
	return matrix * GetMatrix();
}
