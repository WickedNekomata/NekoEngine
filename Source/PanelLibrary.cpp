#include "PanelLibrary.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "imgui\imgui.h"
#include "Brofiler\Brofiler.h"

PanelLibrary::PanelLibrary(char* name) : Panel(name) {}

PanelLibrary::~PanelLibrary() {}

bool PanelLibrary::Draw()
{
	ImGuiWindowFlags libraryFlags = 0;
	libraryFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, libraryFlags))
	{
		if (ImGui::TreeNodeEx(DIR_LIBRARY))
		{
			RecursiveDrawLibraryDir(App->fs->GetRootLibraryFile());
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelLibrary::RecursiveDrawLibraryDir(LibraryFile* libraryFile) const
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	assert(libraryFile != nullptr);

	ImGuiTreeNodeFlags treeNodeFlags;

	char id[DEFAULT_BUF_SIZE];

	for (uint i = 0; i < libraryFile->children.size(); ++i)
	{
		LibraryFile* child = (LibraryFile*)libraryFile->children[i];

		bool treeNodeOpened = false;

		if (child->isDirectory)
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

			sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", child->name.data(), child->path.data());
			if (ImGui::TreeNodeEx(id, treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				SELECT(NULL);

			if (treeNodeOpened)
			{
				if (!child->children.empty())
					RecursiveDrawLibraryDir(child);
				ImGui::TreePop();
			}
		}
		else
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			if (App->scene->selectedObject == child->resource)
				treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

			sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", child->name.data(), child->path.data());
			if (ImGui::TreeNodeEx(id, treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				SELECT(child->resource);

			if (treeNodeOpened)
				ImGui::TreePop();
		}
	}
}

#endif // GAME