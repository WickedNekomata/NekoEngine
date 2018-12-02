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
			RecursiveDrawDir(App->fs->GetRootFileInAssets());
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelAssets::RecursiveDrawDir(FileInAssets* fileInAssets) const
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	assert(fileInAssets != nullptr);

	ImGuiTreeNodeFlags treeNodeFlags;

	for (uint i = 0; i < fileInAssets->children.size(); ++i)
	{
		FileInAssets* child = fileInAssets->children[i];

		std::string extension;
		App->fs->GetExtension(child->name.data(), extension);

		// Ignore metas
		if (!IS_META(extension.data()))
			continue;

		if (!child->children.empty())
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

			bool treeNodeOpened = false;
			if (ImGui::TreeNodeEx(name, treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				RecursiveDrawDir(child);
				ImGui::TreePop();
			}
		}
		else
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			ImGui::TreeNodeEx(name, treeNodeFlags);
			ImGui::TreePop();
		}
		/*
				ResourceType type = ModuleResourceManager::GetResourceTypeByExtension(extension.data());

				if (type != ResourceType::Mesh_Resource)
					treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
				else
					treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

				if (ImGui::TreeNodeEx(*it, treeNodeFlags))
					treeNodeOpened = true;

				if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
					&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				{
						// Search for the meta associated to the file
						char metaFile[DEFAULT_BUF_SIZE];
						strcpy_s(metaFile, strlen(path.data()) + 1, path.data()); // file
						strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

						switch (type)
						{
						case ResourceType::Mesh_Resource:
						{
							MeshImportSettings* currentSettings = new MeshImportSettings();
							App->sceneImporter->GetMeshImportSettingsFromMeta(metaFile, currentSettings);

							// Every dynamic allocation here is deleted at currentSettings

							currentSettings->metaFile = new char[DEFAULT_BUF_SIZE];
							strcpy_s((char*)currentSettings->metaFile, INPUT_BUF_SIZE, metaFile);

							DESTROYANDSET(currentSettings);
							break;
						}
						case ResourceType::Texture_Resource:
						{
							TextureImportSettings* currentSettings = new TextureImportSettings();
							App->materialImporter->GetTextureImportSettingsFromMeta(metaFile, currentSettings);

							// Every dynamic allocation here is deleted at currentSettings

							currentSettings->metaFile = new char[DEFAULT_BUF_SIZE];
							strcpy_s((char*)currentSettings->metaFile, INPUT_BUF_SIZE, metaFile);

							DESTROYANDSET(currentSettings);
							break;
						}
						case ResourceType::No_Type_Resource:

							if (strcmp(extension.data(), EXTENSION_SCENE) == 0)
								DESTROYANDSET(CurrentSelection::SelectedType::scene);

							break;
						}
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
	case ResourceType::No_Type_Resource:

		if (IS_SCENE(extension.data()))
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("DROP_PREFAB_TO_GAME", file, sizeof(char) * (strlen(file) + 1));
				ImGui::EndDragDropSource();
			}
		}
		break;

	case ResourceType::Texture_Resource:

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
		break;
	}
}

#endif // GAME