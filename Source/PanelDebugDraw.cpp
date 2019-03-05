#include "PanelDebugDraw.h"

#ifndef GAMEMODE
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleUI.h"
#include "ModuleScene.h"

#include "ImGui\imgui.h"

PanelDebugDraw::PanelDebugDraw(const char* name) : Panel(name) {}

PanelDebugDraw::~PanelDebugDraw() {}

bool PanelDebugDraw::Draw()
{
	ImGuiWindowFlags debugDrawFlags = 0;
	debugDrawFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, debugDrawFlags))
	{
		bool showGrid = App->scene->GetShowGrid();
		if (ImGui::Checkbox("Grid", &showGrid)) { App->scene->SetShowGrid(showGrid); }

		ImGui::SameLine();

		bool wireframeMode = App->renderer3D->IsWireframeMode();
		if (ImGui::Checkbox("Wireframe", &wireframeMode)) { App->renderer3D->SetWireframeMode(wireframeMode); }

		ImGui::SameLine();

		bool uiMode = App->ui->GetUIMode();
		if (ImGui::Checkbox("Edit or show UI", &uiMode)) { App->ui->SetUIMode(uiMode); }

		ImGui::SameLine();

		ImGui::Checkbox("Debug Draw", &App->renderer3D->debugDraw);

		if (App->renderer3D->debugDraw)
		{
			ImGui::SameLine();
			ImGui::Checkbox("Bounding Boxes", &App->renderer3D->drawBoundingBoxes);
			ImGui::SameLine();
			ImGui::Checkbox("Frustums", &App->renderer3D->drawFrustums);
			ImGui::SameLine();
			ImGui::Checkbox("Colliders", &App->renderer3D->drawColliders);
			ImGui::SameLine();
			ImGui::Checkbox("Rigid Actors", &App->renderer3D->drawRigidActors);
			ImGui::SameLine();
			ImGui::Checkbox("Quadtree", &App->renderer3D->drawQuadtree);
		}
	}
	ImGui::End();

	return true;
}

#endif