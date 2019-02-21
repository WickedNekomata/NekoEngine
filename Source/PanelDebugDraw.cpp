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

		bool debugDraw = App->renderer3D->GetDebugDraw();
		if (ImGui::Checkbox("Debug Draw", &debugDraw)) { App->renderer3D->SetDebugDraw(debugDraw); }

		if (debugDraw)
		{
			ImGui::SameLine();

			bool drawBoundingBoxes = App->renderer3D->GetDrawBoundingBoxes();
			if (ImGui::Checkbox("Bounding Boxes", &drawBoundingBoxes)) { App->renderer3D->SetDrawBoundingBoxes(drawBoundingBoxes); }

			ImGui::SameLine();

			bool drawCamerasFrustum = App->renderer3D->GetDrawCamerasFrustum();
			if (ImGui::Checkbox("Cameras Frustum", &drawCamerasFrustum)) { App->renderer3D->SetDrawCamerasFrustum(drawCamerasFrustum); }

			ImGui::SameLine();

			bool drawColliders = App->renderer3D->GetDrawColliders();
			if (ImGui::Checkbox("Colliders", &drawColliders)) { App->renderer3D->SetDrawColliders(drawColliders); }

			ImGui::SameLine();

			bool drawRigidActors = App->renderer3D->GetDrawRigidActors();
			if (ImGui::Checkbox("Rigid Actors", &drawRigidActors)) { App->renderer3D->SetDrawRigidActors(drawRigidActors); }

			ImGui::SameLine();

			bool drawQuadtree = App->renderer3D->GetDrawQuadtree();
			if (ImGui::Checkbox("Quadtree", &drawQuadtree)) { App->renderer3D->SetDrawQuadtree(drawQuadtree); }
		}
	}
	ImGui::End();

	return true;
}

#endif