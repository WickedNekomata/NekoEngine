#include "PanelPhysics.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModulePhysics.h"
#include "Layers.h"

#include "imgui\imgui.h"

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

		// ----

		if (ImGui::TreeNodeEx("Layer Collision Matrix", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::NewLine();

			std::vector<Layer*> layers = App->layers->GetLayers();
			char layerName[DEFAULT_BUF_SIZE];

			uint pos_x = 55.0f;

			std::vector<Layer*> activeLayers;
			for (int i = layers.size() - 1; i >= 0; --i)
			{
				if (strcmp(layers[i]->name.data(), "") == 0)
					continue;

				ImGui::SameLine(pos_x);

				ImGui::Text("%i", layers[i]->GetNumber());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", layers[i]->name.data());

				activeLayers.push_back(layers[i]);

				pos_x += 27.0f;
			}
			activeLayers.shrink_to_fit();

			int size = activeLayers.size();
			for (int i = activeLayers.size() - 1; i >= 0; --i)
			{
				ImGui::Text("%i", activeLayers[i]->GetNumber());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", activeLayers[i]->name.data());

				pos_x = 55.0f;

				for (uint j = 0; j < size; ++j)
				{
					ImGui::SameLine(pos_x);

					sprintf_s(layerName, DEFAULT_BUF_SIZE, "##%i%i", i, j);
					uint filterMask = activeLayers[i]->filterMask;
					if (ImGui::CheckboxFlags(layerName, &filterMask, activeLayers[j]->GetFilterGroup()))
						activeLayers[i]->filterMask = filterMask;

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%s/%s", activeLayers[i]->name.data(), activeLayers[j]->name.data());

					pos_x += 27.0f;
				}

				--size;
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

#endif