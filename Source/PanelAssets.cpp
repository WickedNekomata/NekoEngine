#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "ModuleResourceManager.h"

#include "imgui\imgui.h"
#include "Brofiler\Brofiler.h"

PanelAssets::PanelAssets(char* name) : Panel(name) {}

PanelAssets::~PanelAssets() {}

bool PanelAssets::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		if (ImGui::Button("Refresh"))
		{
			System_Event newEvent;
			newEvent.type = System_Event_Type::RefreshAssets;
			App->PushSystemEvent(newEvent);
		}

		if (ImGui::TreeNodeEx(DIR_ASSETS))
		{
			RecursiveDrawDir(App->fs->GetRootAssetsFile());
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelAssets::RecursiveDrawDir(AssetsFile* assetsFile) const
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	assert(assetsFile != nullptr);

	ImGuiTreeNodeFlags treeNodeFlags;

	for (uint i = 0; i < assetsFile->children.size(); ++i)
	{
		AssetsFile* child = (AssetsFile*)assetsFile->children[i];

		std::string extension;
		App->fs->GetExtension(child->path.data(), extension);

		// Ignore metas
		if (IS_META(extension.data()))
			continue;

		bool treeNodeOpened = false;

		if (child->isDirectory)
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

			if (ImGui::TreeNodeEx(child->name.data(), treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				if (!child->children.empty())
					RecursiveDrawDir(child);
				ImGui::TreePop();
			}
		}
		else
		{
			ResourceType type = ModuleResourceManager::GetResourceTypeByExtension(extension.data());
			treeNodeFlags = 0;

			if (type != ResourceType::Mesh_Resource)
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			else
				treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

			switch (type)
			{
			case ResourceType::Mesh_Resource:
				if (App->scene->selectedObject == ((MeshImportSettings*)child->importSettings))
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
				break;
			case ResourceType::Texture_Resource:
				if (App->scene->selectedObject == ((TextureImportSettings*)child->importSettings))
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
				break;
			}

			if (ImGui::TreeNodeEx(child->name.data(), treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			{
				switch (type)
				{
				case ResourceType::Mesh_Resource:
					SELECT((MeshImportSettings*)child->importSettings);
					break;
				case ResourceType::Texture_Resource:
					SELECT((TextureImportSettings*)child->importSettings);
					SetResourceDragAndDropSource(type, nullptr, child->UUIDs.begin()->second);
					break;
				case ResourceType::No_Type_Resource:
					if (IS_SCENE(extension.data()))
					{
						SELECT(CurrentSelection::SelectedType::scene);
						SetResourceDragAndDropSource(type, child->path.data());
					}
					else
						SELECT(NULL);
					break;
				}
			}

			if (treeNodeOpened)
			{
				if (type == ResourceType::Mesh_Resource)
				{
					for (std::map<std::string, uint>::const_iterator it = child->UUIDs.begin(); it != child->UUIDs.end(); ++it)
					{
						ImGui::TreeNodeEx(it->first.data(), ImGuiTreeNodeFlags_Leaf);
						ImGui::TreePop();

						SetResourceDragAndDropSource(type, nullptr, it->second);
					}
				}
				ImGui::TreePop();
			}
		}
	}
}

void PanelAssets::SetResourceDragAndDropSource(ResourceType type, const char* file, uint UUID) const
{
	switch (type)
	{
	case ResourceType::Mesh_Resource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::Texture_Resource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::No_Type_Resource:
	{
		std::string extension;
		App->fs->GetExtension(file, extension);

		if (IS_SCENE(extension.data()))
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("DROP_PREFAB_TO_GAME", file, sizeof(char) * (strlen(file) + 1));
				ImGui::EndDragDropSource();
			}
		}
	}
	break;
	}
}

#endif // GAME