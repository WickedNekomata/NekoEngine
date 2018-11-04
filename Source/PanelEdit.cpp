#include "PanelEdit.h"

#ifndef GAMEMODE
#include "Application.h"
#include "ModuleGui.h"

#include "ImGui/imgui.h"

PanelEdit::PanelEdit(char* name) : Panel(name) {}

PanelEdit::~PanelEdit() {}

bool PanelEdit::Draw()
{
	Texture* atlas = App->gui->atlas;

	if (atlas == nullptr)
		return false;
	
	float itemScale = 0.7f;
	float itemSize = 1.0f / 4.0f;
	ImVec2 imageSize(atlas->width * itemSize * itemScale, atlas->height * itemSize * itemScale);

	ImGuiWindowFlags editFlags = 0;
	editFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, editFlags))
	{
		// 1. Gizmos

		// Move Tool button
		if (App->scene->GetImGuizmoOperation() == ImGuizmo::OPERATION::TRANSLATE)
		{
			ImGui::PushID("moveHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(0.0f, itemSize), ImVec2(itemSize, itemSize * 2.0f));

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("moveNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(0.0f, itemSize * 3.0f), ImVec2(itemSize, itemSize * 4.0f)))
				App->scene->SetImGuizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Move Tool");
			ImGui::PopID();
		}
		ImGui::SameLine();

		// Rotate Tool button
		if (App->scene->GetImGuizmoOperation() == ImGuizmo::OPERATION::ROTATE)
		{
			ImGui::PushID("rotateHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize, itemSize), ImVec2(itemSize * 2.0f, itemSize * 2.0f));

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("rotateNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize, itemSize * 3.0f), ImVec2(itemSize * 2.0f, itemSize * 4.0f)))
				App->scene->SetImGuizmoOperation(ImGuizmo::OPERATION::ROTATE);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Rotate Tool");
			ImGui::PopID();
		}
		ImGui::SameLine();

		// Scale Tool button
		if (App->scene->GetImGuizmoOperation() == ImGuizmo::OPERATION::SCALE)
		{
			ImGui::PushID("scaleHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize * 2.0f, itemSize), ImVec2(itemSize * 3.0f, itemSize * 2.0f));

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("scaleNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize * 2.0f, itemSize * 3.0f), ImVec2(itemSize * 3.0f, itemSize * 4.0f)))
				App->scene->SetImGuizmoOperation(ImGuizmo::OPERATION::SCALE);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Scale Tool");
			ImGui::PopID();
		}

		// 2. Time Management

		float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
		float padding = ImGui::GetStyle().WindowPadding.x;
		float windowCenter = (ImGui::GetWindowContentRegionWidth() - padding) / 2.0f;

		float nextItemsWidth = 3.0f * imageSize.x + 2.0f * itemSpacing;
		float nextItemPosition = windowCenter - (nextItemsWidth / 2.0f);

		ImGui::SameLine(nextItemPosition);

		engine_states engineState = App->GetEngineState();

		// Play button
		if (App->IsPlay() || engineState == engine_states::ENGINE_WANTS_PLAY
			|| App->IsPause() || engineState == engine_states::ENGINE_WANTS_PAUSE
			|| App->IsStep() || engineState == engine_states::ENGINE_WANTS_STEP)
		{
			ImGui::PushID("playHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(0.0f, 0.0f), ImVec2(itemSize, itemSize));

			if (ImGui::IsItemClicked(0))
				App->Play();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("playNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(0.0f, itemSize * 2.0f), ImVec2(itemSize, itemSize * 3.0f)))
				App->Play();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Play");
			ImGui::PopID();
		}
		ImGui::SameLine();

		// Pause button
		if (App->IsPause() || engineState == engine_states::ENGINE_WANTS_PAUSE
			|| App->IsStep() || engineState == engine_states::ENGINE_WANTS_STEP)
		{
			ImGui::PushID("pauseHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize, 0.0f), ImVec2(itemSize * 2.0f, itemSize));

			if (ImGui::IsItemClicked(0))
				App->Pause();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("pauseNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize, itemSize * 2.0f), ImVec2(itemSize * 2.0f, itemSize * 3.0f)))
				App->Pause();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Pause");
			ImGui::PopID();
		}
		ImGui::SameLine();

		// Step button
		if (App->IsStep() || engineState == engine_states::ENGINE_WANTS_STEP)
		{
			ImGui::PushID("stepHovered");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize * 2.0f, 0.0f), ImVec2(itemSize * 3.0f, itemSize));

			if (ImGui::IsItemClicked(0))
				App->Step();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			ImGui::PushID("stepNormal");
			if (ImGui::ImageButton((ImTextureID)atlas->id, imageSize, ImVec2(itemSize * 2.0f, itemSize * 2.0f), ImVec2(itemSize * 3.0f, itemSize * 3.0f)))
				App->Step();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Step");
			ImGui::PopID();
		}
		ImGui::SameLine();

		// Game time scale slider
		ImGui::PushItemWidth(100);
		float timeScale = App->timeManager->GetTimeScale();
		if (ImGui::SliderFloat("Game Time Scale", &timeScale, 0.0f, MAX_TIME_SCALE)) { App->timeManager->SetTimeScale(timeScale); }
	}
	ImGui::End();

	return true;
}

#endif