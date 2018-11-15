#include "PanelResources.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"

#include "ImGui/imgui.h"

PanelResources::PanelResources(char* name) : Panel(name) {}

PanelResources::~PanelResources() {}

bool PanelResources::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		if (ImGui::TreeNodeEx("Library"))
		{
			std::string currentFile;
			RecursiveDrawDir("Library", currentFile);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelResources::RecursiveDrawDir(const char* dir, std::string& currentFile) const
{
	// TODO MODIFY THIS RECURSIVITY
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	currentFile.append(dir);
	currentFile.append("/");

	const char** files = App->fs->GetFilesFromDir(dir);
	const char** it;
	
	for (it = files; *it != nullptr; ++it)
	{
		std::string lib("Lib");
		lib += *it;

		bool treeNodeOpened = false;

		treeNodeFlags = 0;
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
		
		if (App->fs->IsDirectory(lib.data()))
		{
			if (ImGui::TreeNodeEx(*it, treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				
				RecursiveDrawDir(lib.data(), currentFile);
				uint found = currentFile.rfind(*it);
				if (found != std::string::npos)
					currentFile = currentFile.substr(0, found);

				ImGui::TreePop();
			}
		}
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);

			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			uint res_id = strtoul(*it, NULL, 0);
			const Resource* res = App->res->GetResource(res_id);
			
			if (App->scene->selectedObject != NULL && App->scene->selectedObject == res)
				treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(*it, treeNodeFlags);			
			
			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				DESTROYANDSET(res);

			ImGui::TreePop();
			if (res != nullptr) {
				if (res->GetType() == ResourceType::Mesh_Resource) {
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &res_id, sizeof(uint));
						ImGui::EndDragDropSource();
					}
				}
				else if (res->GetType() == ResourceType::Texture_Resource)
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &res_id, sizeof(uint));
						ImGui::EndDragDropSource();
					}
				}
			}
		}
	}
}

#endif // GAME