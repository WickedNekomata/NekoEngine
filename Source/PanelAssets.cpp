#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"

#include "ImGui/imgui.h"

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
		float checkTime = App->res->GetAssetsCheckTime();
		if (ImGui::SliderFloat("Check Time", &checkTime, 0.0f, MAX_ASSETS_CHECK_TIME)) { App->res->SetAssetsCheckTime(checkTime); }

		if (ImGui::TreeNodeEx("Assets"))
		{
			std::string metaPath;
			RecursiveDrawDir("Assets", metaPath);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelAssets::RecursiveDrawDir(const char* dir, std::string& currentFile) const
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	currentFile.append(dir);
	currentFile.append("/");

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
				RecursiveDrawDir(*it, currentFile);
				uint found = currentFile.rfind(*it);
				if (found != std::string::npos)
					currentFile = currentFile.substr(0, found);

				ImGui::TreePop();
			}
		}
		else
		{
			treeNodeFlags = 0;
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

			std::string extension;
			App->filesystem->GetExtension(*it, extension);

			// Ignore assets that generate scenes and metas. Define this
			if (strcmp(extension.data(), ".fbx") == 0 || strcmp(extension.data(), ".FBX") == 0
				|| strcmp(extension.data(), ".obj") == 0 || strcmp(extension.data(), ".OBJ") == 0
				|| strcmp(extension.data(), ".meta") == 0 || strcmp(extension.data(), ".META") == 0)
				continue;

			// Search for the meta associated to the file
			char metaFile[DEFAULT_BUF_SIZE];
			strcpy_s(metaFile, strlen(currentFile.data()) + 1, currentFile.data()); // path
			strcat_s(metaFile, strlen(metaFile) + strlen(*it) + 1, *it); // fileName
			const char metaExtension[] = ".meta";
			strcat_s(metaFile, strlen(metaFile) + strlen(metaExtension) + 1, metaExtension); // extension

			// TODO GUILLEM
			MeshImportSettings* currentSettings = new MeshImportSettings();
			App->sceneImporter->GetMeshImportSettingsFromMeta(metaFile, currentSettings);
			DESTROYANDSET(currentSettings);

			ImGui::TreeNodeEx(*it, treeNodeFlags);
			ImGui::TreePop();
		}
	}
}

#endif // GAME