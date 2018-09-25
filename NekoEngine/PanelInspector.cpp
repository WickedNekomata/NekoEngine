#include "PanelInspector.h"
#include "Globals.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name)
{
}

PanelInspector::~PanelInspector()
{

}

bool PanelInspector::Draw()
{
	ImGui::SetNextWindowPos({ SCREEN_WIDTH - 400,20 });
	ImGui::SetNextWindowSize({ 400,400 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin(name, false, inspectorFlags);
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::Text("Position");
		ImGui::SameLine();
		static int posX = 10;
		static int posY = 10;
		static int posZ = 10;
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posX, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posY, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posZ, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("RigidBody"))
	{
		ImGui::Text("Coming soon...");
	}
	ImGui::End();

	return true;
}
