#include "PanelImport.h"

#include "ImGui/imgui.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "SceneImporter.h"

#include "Globals.h"

PanelImport::PanelImport(char* name) : Panel(name) {}

PanelImport::~PanelImport() {}

bool PanelImport::Draw()
{
	ImGui::SetNextWindowSize({ 400,400 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags importFlags = 0;
	importFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	
	if (ImGui::Begin(name, &enabled, importFlags))
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
		if (ImGui::Button("Import file")) { App->sceneImporter->LoadMeshesWithPHYSFS(file.c_str()); }
	}
	ImGui::End();

	return true;
}