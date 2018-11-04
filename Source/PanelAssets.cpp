#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"

#include "ImGui/imgui.h"

PanelAssets::PanelAssets(char* name) : Panel(name) {}

PanelAssets::~PanelAssets() {}

bool PanelAssets::Draw()
{
	ImGui::SetNextWindowPos({ (float)App->window->GetWindowWidth() - 300,50 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300,(float)App->window->GetWindowHeight() - 50 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	assetsFlags |= ImGuiWindowFlags_NoSavedSettings;
	assetsFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;

	if (ImGui::Begin(name, &enabled))
	{
		if (ImGui::TreeNodeEx("Assets"))
		{
			RecursiveDrawDir("Assets");
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelAssets::RecursiveDrawDir(const char* dir)
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	const char** files = App->filesystem->GetFilesFromDir(dir);
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		bool treeNodeOpened = false;

		treeNodeFlags = 0;
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

		if (App->filesystem->IsDirectory(*it))
		{
			if (ImGui::TreeNodeEx(*it, treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				RecursiveDrawDir(*it);
				ImGui::TreePop();
			}
		}		
		else
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			ImGui::TreeNodeEx(*it, treeNodeFlags);
			ImGui::TreePop();
		}
	}
}

#endif // GAME