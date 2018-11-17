#include "PanelResources.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"

#include "ImGui\imgui.h"
#include "Brofiler\Brofiler.h"

PanelResources::PanelResources(char* name) : Panel(name) {}

PanelResources::~PanelResources() {}

bool PanelResources::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		if (ImGui::TreeNodeEx(DIR_LIBRARY))
		{
			std::string path = DIR_LIBRARY;
			RecursiveDrawDir(DIR_LIBRARY, path);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelResources::RecursiveDrawDir(const char* dir, std::string& path) const
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return;
	}

	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	const char** files = App->fs->GetFilesFromDir(path.data());
	const char** it;

	path.append("/");
	
	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		bool treeNodeOpened = false;

		treeNodeFlags = 0;
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
		
		if (App->fs->IsDirectory(path.data()))
		{
			if (ImGui::TreeNodeEx(*it, treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				RecursiveDrawDir(*it, path);
				ImGui::TreePop();
			}
		}
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);

			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			uint UUID = strtoul(*it, NULL, 0);
			const Resource* res = App->res->GetResource(UUID);
			
			if (App->scene->selectedObject != NULL && App->scene->selectedObject == res)
				treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(*it, treeNodeFlags);			
			
			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				DESTROYANDSET(res);

			ImGui::TreePop();

			if (res != nullptr) 
			{
				if (res->GetType() == ResourceType::Mesh_Resource) 
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
						ImGui::EndDragDropSource();
					}
				}
				else if (res->GetType() == ResourceType::Texture_Resource)
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
						ImGui::EndDragDropSource();
					}
				}
			}
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

#endif // GAME