#include "PanelImport.h"

#include "ImGui/imgui.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleAssetImporter.h"

#include "Globals.h"

PanelImport::PanelImport(char* name) : Panel(name) {}

PanelImport::~PanelImport() {}

bool PanelImport::Draw()
{
	ImGui::SetNextWindowSize({ 200,120 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoSavedSettings;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	
	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		// TODO: Get PHYSFS paths for combo
		ImGui::Text("Base Path:");
		const char* items[] = { "Assets/", "Assets/Model", "." };
		App->filesystem->GetReadPaths();
		static int item_current = 0;
		ImGui::Combo("", &item_current, items, IM_ARRAYSIZE(items));
		static char fileNameBuf[STR_INPUT_SIZE];
		ImGui::InputText("File", fileNameBuf, IM_ARRAYSIZE(fileNameBuf));

		std::string file = items[item_current];
		file += fileNameBuf;
		if (ImGui::Button("Import file")) { App->assetsImporter->LoadFBXwithPHYSFS(file.c_str()); }
		ImGui::End();
	}
	return true;
}