#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"

#include "ImGui\imgui.h"
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
		App->fs->GetExtension(child->name.data(), extension);

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

			if (ImGui::TreeNodeEx(child->name.data(), treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			{
				switch (type)
				{
				case ResourceType::Mesh_Resource:
					DESTROYANDSET((MeshImportSettings*)child->importSettings);
					break;
				case ResourceType::Texture_Resource:
					DESTROYANDSET((TextureImportSettings*)child->importSettings);
					break;
				case ResourceType::No_Type_Resource:
					if (IS_SCENE(extension.data()))
					{
						DESTROYANDSET(CurrentSelection::SelectedType::scene);
					}
					else
						DESTROYANDSET(CurrentSelection::SelectedType::null);
					break;
				}
			}
			ImGui::TreePop();
			/*
			if (treeNodeOpened)
			{
				if (type == ResourceType::Mesh_Resource)
				{
					for (std::list<uint>)
				}

				ImGui::TreePop();
			}
			*/
		}
		/*
				

				if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
					&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				{
				
				}

				if (treeNodeOpened)
				{
						// Search for the meta associated to the file
						char metaFile[DEFAULT_BUF_SIZE];
						strcpy_s(metaFile, strlen(path.data()) + 1, path.data()); // file
						strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

						std::list<uint> UUIDs;
						App->sceneImporter->GetMeshesUUIDsFromMeta(metaFile, UUIDs);

						for (auto it = UUIDs.begin(); it != UUIDs.end(); ++it)
						{
							ImGui::TreeNodeEx(std::to_string(*it).data(), ImGuiTreeNodeFlags_Leaf);
							ImGui::TreePop();

							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
							{
								ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &(*it), sizeof(uint));
								ImGui::EndDragDropSource();
							}
						}
					}
					ImGui::TreePop();
				}

				SetDragAndDropSource(path.data());
			}
	}
	*/
	}
}

void PanelAssets::SetDragAndDropSource(const char* file) const
{
	std::string extension;
	App->fs->GetExtension(file, extension);

	switch (ModuleResourceManager::GetResourceTypeByExtension(extension.data()))
	{
	case ResourceType::Texture_Resource:
	{
		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(file) + 1, file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		uint UUID = 0;
		App->materialImporter->GetTextureUUIDFromMeta(metaFile, UUID);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
			ImGui::EndDragDropSource();
		}
	}
	break;
	case ResourceType::No_Type_Resource:
	{
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