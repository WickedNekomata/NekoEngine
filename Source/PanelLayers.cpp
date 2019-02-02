#include "PanelLayers.h"

#ifndef GAMEMODE

#include "Application.h"
#include "Layers.h"

#include "ImGui\imgui.h"
#include "imgui\imgui_internal.h"

PanelLayers::PanelLayers(const char* name) : Panel(name) {}

PanelLayers::~PanelLayers() {}

bool PanelLayers::Draw()
{
	ImGuiWindowFlags aboutFlags = 0;
	aboutFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, aboutFlags))
	{
		const ImVec4 disabledTextColor = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		char id[DEFAULT_BUF_SIZE];
		char layerName[INPUT_BUF_SIZE];

		for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
		{
			if (App->layers->NumberToBuiltin(i))
			{
				ImGui::TextColored(disabledTextColor, "Builtin Layer %i", i); ImGui::SameLine(150.0f);
				strcpy_s(layerName, INPUT_BUF_SIZE, App->layers->NumberToName(i));
				sprintf_s(id, DEFAULT_BUF_SIZE, "##BuiltinLayer%i", i);
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushItemWidth(200.0f);
				ImGui::InputText(id, layerName, INPUT_BUF_SIZE);
				ImGui::PopItemWidth();
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
			}
			else
			{
				ImGui::Text("User Layer %i", i); ImGui::SameLine(150.0f);
				strcpy_s(layerName, INPUT_BUF_SIZE, App->layers->NumberToName(i));
				sprintf_s(id, DEFAULT_BUF_SIZE, "##UserLayer%i", i);
				ImGui::PushItemWidth(200.0f);
				if (ImGui::InputText(id, layerName, INPUT_BUF_SIZE))
					App->layers->SetLayerName(i, layerName);
				ImGui::PopItemWidth();
			}
		}
	}
	ImGui::End();

	return true;
}

#endif