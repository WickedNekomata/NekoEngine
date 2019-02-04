#include "PanelPhysics.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModulePhysics.h"

#include "ImGui\imgui.h"

PanelPhysics::PanelPhysics(const char* name) : Panel(name) {}

PanelPhysics::~PanelPhysics() {}

bool PanelPhysics::Draw()
{
	ImGuiWindowFlags aboutFlags = 0;
	aboutFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, aboutFlags))
	{
		ImGui::Text("Gravity");
		math::float3 gravity = App->physics->GetGravity();

		ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##GravityX", &gravity.x, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			App->physics->SetGravity(gravity);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if (ImGui::DragFloat("##GravityY", &gravity.y, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
			App->physics->SetGravity(gravity);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("##GravityZ", &gravity.z, 0.01f, -FLT_MAX, FLT_MAX, "%.2f", 1.0f))
				App->physics->SetGravity(gravity);
		ImGui::PopItemWidth();

		// -----

		ImGui::Text("Default Material");
		ImGui::SameLine();

		ImGui::PushID("Material");
		const physx::PxMaterial* material = App->physics->GetDefaultMaterial();
		ImGui::Button("Default Material", ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PHYSICS_MATERIAL"))
			{
				//uint payload_n = *(uint*)payload->Data;
				//SetResource(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
	ImGui::End();

	return true;
}

#endif