#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name)
{
}

PanelInspector::~PanelInspector()
{

}

bool PanelInspector::Draw()
{
	ImGui::SetNextWindowPos({ (float)App->window->GetWindowWidth() - 400,20 });
	ImGui::SetNextWindowSize({ 400,400 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin(name, &enabled, inspectorFlags);
	ImGui::Spacing();
	

	if (ImGui::CollapsingHeader("Meshes"))
	{
		int numMeshes = App->renderer3D->GetNumMeshes();
		ImGui::Text("Meshes: %i", numMeshes);

		for (int i = 0; i < numMeshes; ++i)
		{
			Mesh* mesh = App->renderer3D->GetMeshByIndex(i);
			ImGui::TextColored(ImVec4(60, 255, 255, 1), "Mesh %i", i + 1);
			ImGui::Text("Vertex: %i", mesh->verticesSize / 3);
			ImGui::Text("Vertex ID: %i", mesh->verticesID);
			ImGui::Text("Texture Coords: %i", mesh->textureCoordsID);
			ImGui::Separator();
		}
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Texture"))
	{
		Mesh* mesh = App->renderer3D->GetMeshByIndex(0);
		if (mesh != nullptr)
		{
			if (mesh->textureID != 0)
			{
				ImGui::Image((void*)(intptr_t)mesh->textureID, ImVec2(128, 128));
				ImGui::Text("Texture ID: %i", mesh->textureID);
				ImGui::Text("Width: %i\nHeight: %i", mesh->textureWidth, mesh->textureHeight);
			}
		}
	}
	ImGui::End();

	return true;
}
