#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGui::SetNextWindowSize({ 400,400 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		Mesh* mesh = App->renderer3D->GetMeshByIndex(0);

		if (ImGui::CollapsingHeader("Transform"))
		{
			if (mesh != nullptr)
			{
				ImGui::Text("Position");
				math::float3 position = mesh->position;
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##posX", &position.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##posY", &position.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##posZ", &position.z);

				ImGui::Text("Rotation");
				math::Quat rotationQuat = mesh->rotation;
				math::float3 rotationEuler = rotationQuat.ToEulerXYZ();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##rotX", &rotationEuler.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##rotY", &rotationEuler.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##rotZ", &rotationEuler.z);

				ImGui::Text("Scale");
				math::float3 scale = mesh->scale;
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##scaleX", &scale.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##scaleY", &scale.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##scaleZ", &scale.z);
			}
		}

		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Geometry"))
		{
			uint numMeshes = App->renderer3D->GetNumMeshes();
			ImGui::Text("Meshes: %i", numMeshes);

			for (int i = 0; i < numMeshes; ++i)
			{
				Mesh* mesh = App->renderer3D->GetMeshByIndex(i);
				ImGui::TextColored(WHITE, "Mesh %i", i + 1);

				ImGui::Text("Vertices: %i", mesh->verticesSize);
				ImGui::Text("Vertices ID: %i", mesh->verticesID);

				ImGui::Text("Indices: %i", mesh->indicesSize);
				ImGui::Text("Indices ID: %i", mesh->indicesID);

				ImGui::Text("Texture Coords: %i", mesh->verticesSize);
				ImGui::Text("Texture Coords ID: %i", mesh->textureCoordsID);

				ImGui::Separator();
			}
		}

		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Material"))
		{
			if (mesh != nullptr)
			{
				if (mesh->textureID != 0)
				{
					ImGui::Text("Textures: 1");

					ImGui::Image((void*)(intptr_t)mesh->textureID, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::Text("Texture ID: %i", mesh->textureID);
					ImGui::Text("Width: %i", mesh->textureWidth);
					ImGui::Text("Height %i", mesh->textureHeight);
				}
			}
			else
				ImGui::Text("Textures: 0");
		}
	}
	ImGui::End();

	return true;
}
