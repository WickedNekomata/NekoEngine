#include "PanelEdit.h"

#include "Application.h"
#include "ModuleGui.h"

#include "ImGui/imgui.h"

PanelEdit::PanelEdit(char* name) : Panel(name) {}

PanelEdit::~PanelEdit() {}

bool PanelEdit::Draw()
{
	ImGuiWindowFlags editFlags = 0;
	editFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, editFlags))
	{
		Texture* timeTex = App->gui->timeTex;
		float timeButtonScale = 0.05f;
		ImVec2 timeButtonSize(timeTex->width * 0.2f * timeButtonScale, timeTex->height * 1.0f * timeButtonScale);

		// Play button
		if (App->IsPlay() || App->IsPause())
		{
			ImGui::PushID("play");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)timeTex->id, timeButtonSize, ImVec2(0.4f, 0.0f), ImVec2(0.6f, 1.0f));

			if (ImGui::IsItemClicked(0))
				App->Play();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			if (ImGui::ImageButton((ImTextureID)timeTex->id, timeButtonSize, ImVec2(0.4f, 0.0f), ImVec2(0.6f, 1.0f)))
				App->Play();
		}
		ImGui::SameLine();

		// Pause button
		if (App->IsPause())
		{
			ImGui::PushID("pause");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)timeTex->id, timeButtonSize, ImVec2(0.0f, 0.0f), ImVec2(0.2f, 1.0f));

			if (ImGui::IsItemClicked(0))
				App->Pause();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			if (ImGui::ImageButton((ImTextureID)timeTex->id, timeButtonSize, ImVec2(0.0f, 0.0f), ImVec2(0.2f, 1.0f)))
				App->Pause();
		}
		ImGui::SameLine();

		// Step button
		ImGui::PushID("tick");
		if (ImGui::ImageButton((ImTextureID)timeTex->id, timeButtonSize, ImVec2(0.6f, 0.0f), ImVec2(0.8f, 1.0f)))
			App->Tick();
		ImGui::PopID();

		// Game time scale slider
		ImGui::PushItemWidth(100);
		float timeScale = App->timeManager->GetTimeScale();
		if (ImGui::SliderFloat("Game Time Scale", &timeScale, 0.0f, MAX_TIME_SCALE)) { App->timeManager->SetTimeScale(timeScale); }
	}
	ImGui::End();

	return true;
}