#include "PanelAbout.h"

#include "Globals.h"

#include "ImGui/imgui.h"

PanelAbout::PanelAbout(char* name) : Panel(name)
{
}


PanelAbout::~PanelAbout()
{
}

bool PanelAbout::Draw()
{
	CONSOLE_LOG("Hello");
	ImGui::SetNextWindowSize({ 350,250 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin(name, &enabled, inspectorFlags);
	ImGui::Text("Neko Engine");
	ImGui::Spacing();
	ImGui::Text("This is a 3D game engine being developed by\ntwo students from CITM-UPC Terrassa.");
	ImGui::Separator();
	ImGui::Text("Authors:");
	if (ImGui::Button("Sandra Alvarez")) { OpenInBrowser("https://github.com/Sandruski"); }
	if (ImGui::Button("Guillem Costa")) { OpenInBrowser("https://github.com/DatBeQuiet"); }
	ImGui::Separator();
	ImGui::Text("Libraries:");
	ImGui::Text("ImGuI, OpenGL 2.0, \nSDL 2.0, MathGeoLib, PCG");
	ImGui::Separator();
	ImGui::Text("MIT License");
	ImGui::End();

	return true;
}