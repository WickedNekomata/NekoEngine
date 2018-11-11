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
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	currentFile.append(dir);
	currentFile.append("/");

	const char** files = App->fs->GetFilesFromDir(dir);
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		bool treeNodeOpened = false;

		treeNodeFlags = 0;
		treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

		if (App->fs->IsDirectory(*it))
		{
			if (ImGui::TreeNodeEx(*it, treeNodeFlags))
				treeNodeOpened = true;

			if (treeNodeOpened)
			{
				RecursiveDrawDir(*it, currentFile);
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

			// Ignore assets that generate scenes and metas
			if (IS_MESH_RESOURCE(extension.data()) || IS_META(extension.data()))
				continue;

			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			/*
			if (App->scene->selectedObject == CurrentSelection::SelectedType::meshImportSettings
				|| App->scene->selectedObject == CurrentSelection::SelectedType::textureImportSettings)
			{
				MeshImportSettings* currentSettings = (MeshImportSettings*)(App->scene->selectedObject.Get());
				// TODO: get file name and compare. if equals set next treenode as selected :)
			}
			*/
			ImGui::TreeNodeEx(*it, treeNodeFlags);

			if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			{
				// Search for the meta associated to the file
				char metaFile[DEFAULT_BUF_SIZE];
				strcpy_s(metaFile, strlen(currentFile.data()) + 1, currentFile.data()); // path
				strcat_s(metaFile, strlen(metaFile) + strlen(*it) + 1, *it); // fileName
				const char metaExtension[] = ".meta";
				strcat_s(metaFile, strlen(metaFile) + strlen(metaExtension) + 1, metaExtension); // extension

				ResourceType type;
				if (strcmp(extension.data(), ".nekoScene") == 0)
					type = ResourceType::Mesh_Resource;
				else if (strcmp(extension.data(), ".dds") == 0 || strcmp(extension.data(), ".DDS") == 0
					|| strcmp(extension.data(), ".png") == 0 || strcmp(extension.data(), ".PNG") == 0
					|| strcmp(extension.data(), ".jpg") == 0 || strcmp(extension.data(), ".JPG") == 0)
					type = ResourceType::Texture_Resource;

				switch (type)
				{
				case ResourceType::Mesh_Resource:
				{
					MeshImportSettings* currentSettings = new MeshImportSettings();
					App->sceneImporter->GetMeshImportSettingsFromMeta(metaFile, currentSettings);
					DESTROYANDSET(currentSettings);
					break;
				}
				case ResourceType::Texture_Resource:
				{
					TextureImportSettings* currentSettings = new TextureImportSettings();
					App->materialImporter->GetTextureImportSettingsFromMeta(metaFile, currentSettings);
					DESTROYANDSET(currentSettings);
					break;
				}
				}
			}
			ImGui::TreePop();
		}
	}
}

#endif // GAME