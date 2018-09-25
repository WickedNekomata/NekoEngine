#include "PanelAbout.h"

#include "ImGui/imgui.h"

PanelAbout::PanelAbout(char* name) : Panel(name)
{
}


PanelAbout::~PanelAbout()
{
}

bool PanelAbout::Draw()
{
	ImGui::SetNextWindowSize({ 250,200 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin(name, &enabled, inspectorFlags);
	ImGui::Text("Neko Engine");
	ImGui::Spacing();
	ImGui::Text("This is an engine developed by 2\nstudent at CITM-UPC Terrassa");
	ImGui::Separator();
	ImGui::Text("Authors: \nSandra Alvarez Garcia\nGuillem Costa Miquel");
	ImGui::Separator();
	ImGui::Text("Libraries: ImGuI, OpenGL 2.0,\nSDL 2.0, MathGeoLib, PCG");
	ImGui::Separator();
	ImGui::Text("MIT License");
	ImGui::End();

	return true;
}