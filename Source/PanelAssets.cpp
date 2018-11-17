#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"

#include "ImGui/imgui.h"
#include "Brofiler/Brofiler.h"

PanelAssets::PanelAssets(char* name) : Panel(name) {}

PanelAssets::~PanelAssets() {}

bool PanelAssets::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		// Game time scale slider
		ImGui::PushItemWidth(100.0f);
		float checkTime = App->fs->GetAssetsCheckTime();
		if (ImGui::SliderFloat("Check Time", &checkTime, 0.0f, MAX_ASSETS_CHECK_TIME)) { App->fs->SetAssetsCheckTime(checkTime); }

		if (ImGui::TreeNodeEx(DIR_ASSETS))
		{
			std::string path = DIR_ASSETS;
			RecursiveDrawDir(DIR_ASSETS, path);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelAssets::RecursiveDrawDir(const char* dir, std::string& path) const
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

			// Ignore metas
			if (!IS_META(extension.data()))
			{
				treeNodeFlags = 0;

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
					// Ignore scenes
					if (!IS_SCENE(extension.data()))
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
				}

				if (treeNodeOpened)
				{
					// Ignore scenes
					if (!IS_SCENE(extension.data()))
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

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
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