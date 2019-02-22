#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "ShaderImporter.h"
#include "ModuleInternalResHandler.h"

#include "imgui\imgui.h"
#include "Brofiler\Brofiler.h"

#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ScriptingModule.h"

PanelAssets::PanelAssets(const char* name) : Panel(name) {}

PanelAssets::~PanelAssets() {}

bool PanelAssets::Draw()
{
	ImGuiWindowFlags assetsFlags = 0;
	assetsFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, assetsFlags))
	{
		if (ImGui::Button("Re-Import"))
		{
			App->scripting->CreateDomain();

			App->fs->ForceReImport(App->fs->rootAssets);

			System_Event event;
			event.type = System_Event_Type::DeleteUnusedFiles;
			App->PushSystemEvent(event);
		}
		else if (ImGui::Button("Copy Shaders Into Library"))
		{
			System_Event newEvent;
			newEvent.type = System_Event_Type::CopyShadersIntoLibrary;
			App->PushSystemEvent(newEvent);
		}

		bool treeNodeOpened = ImGui::TreeNodeEx(DIR_ASSETS);
		CreateShaderPopUp(DIR_ASSETS);
		if (treeNodeOpened)
		{
			RecursiveDrawAssetsDir(App->fs->rootAssets);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (showCreateShaderConfirmationPopUp)
	{
		ImGui::OpenPopup("Create Shader");
		CreateShaderConfirmationPopUp();
	}
	else if (showDeleteShaderConfirmationPopUp)
	{
		ImGui::OpenPopup("Delete Shader");
		DeleteShaderConfirmationPopUp();
	}

	return true;
}

void PanelAssets::RecursiveDrawAssetsDir(const Directory& directory)
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif

	//TODO: ORGANIZE THIS LOGIC INTO THE OWN ONPANELASSETS METHOD:

	//	1* The stantard draw
	//	2* Manage the selection and show the import settings
	//	3* Drag and Drop support

	for (uint i = 0; i < directory.files.size(); ++i)
	{
		File file = directory.files[i];

		std::string metaFile = directory.fullPath + "//" + file.name + ".meta";
		if (!App->fs->Exists(metaFile))
			continue;

		char* metaBuffer;
		uint metaSize = App->fs->Load(metaFile, &metaBuffer);
		if (metaSize <= 0)
			continue;

		char* cursor = metaBuffer;

		std::string extension;
		App->fs->GetExtension(file.name.data(), extension);

		union
		{
			char ext[4];
			uint32_t asciiValue;
		} asciiUnion;

		memcpy(asciiUnion.ext, extension.data(), sizeof(char) * 4);

		switch (asciiUnion.asciiValue)
		{
			case ASCIIFBX:
			case ASCIIfbx:
			case ASCIIOBJ:
			case ASCIIobj:
			case ASCIIdae:
			case ASCIIDAE:
			{		
				ImGuiTreeNodeFlags flags = 0;
				flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

				if (App->scene->selectedObject == CurrentSelection::SelectedType::meshImportSettings)
				{
					ResourceMeshImportSettings* importSettings = (ResourceMeshImportSettings*)App->scene->selectedObject.Get();
					
					if (strstr(importSettings->modelPath, file.name.data()) != nullptr)
						flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

				}

				char id[DEFAULT_BUF_SIZE];
				sprintf(id, "%s##%s", file.name.data(), directory.fullPath.data());

				bool fbxOpened = ImGui::TreeNodeEx(id, flags);

				ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0);
				if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
				{		
					std::vector<uint> uids;
					ResourceMesh::ReadMeshesUuidsFromBuffer(cursor, uids);

					ResourceMesh* tempRes = (ResourceMesh*)App->res->GetResource(uids[0]);
					SELECT(tempRes->GetSpecificData().meshImportSettings);
				}
			
				if(fbxOpened)
				{
					std::vector<uint> uids;
					ResourceMesh::ReadMeshesUuidsFromBuffer(cursor, uids);

					for (int i = 0; i < uids.size(); ++i)
					{
						Resource* res = (Resource*)App->res->GetResource(uids[i]);
						if (res)
							res->OnPanelAssets();
					}
					ImGui::TreePop();
				}
			
				break;
			}

			default:
			{
				uint uid;
				cursor += sizeof(int64_t);
				cursor += sizeof(uint);
				
				memcpy(&uid, cursor, sizeof(uint));

				Resource* res = (Resource*)App->res->GetResource(uid);
				if(res)
					res->OnPanelAssets();

				break;
			}
		}

		delete[] metaBuffer;
	}

	for (uint i = 0; i < directory.directories.size(); ++i)
	{
		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		Directory dir = directory.directories[i];

		bool treeNodeOpened = false;

		char id[DEFAULT_BUF_SIZE];
		sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", dir.name.data(), dir.fullPath.data());

		if (ImGui::TreeNodeEx(id, flags))
			treeNodeOpened = true;

		//TODO: WHY ARE DIRECTORIES BEING SELECTED?
		if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
			&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			SELECT(NULL);

		//TODO: WHAT IS THIS?
		CreateShaderPopUp(dir.fullPath.data());

		if (treeNodeOpened)
		{
			if (!(dir.files.empty() && dir.directories.empty()))
				RecursiveDrawAssetsDir(dir);
			ImGui::TreePop();
		}
	}
}

void PanelAssets::CreateShaderPopUp(const char* path)
{
	if (ImGui::BeginPopupContextItem(path))
	{
		if (ImGui::Selectable("Create Vertex Shader"))
		{
			shaderType = ShaderTypes::VertexShaderType;
			strcpy_s(shaderName, strlen("New Vertex Shader") + 1, "New Vertex Shader");
			shaderFile = path;
			shaderFile.append("/");

			showCreateShaderConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Fragment Shader"))
		{
			shaderType = ShaderTypes::FragmentShaderType;
			strcpy_s(shaderName, strlen("New Fragment Shader") + 1, "New Fragment Shader");
			shaderFile = path;
			shaderFile.append("/");

			showCreateShaderConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::DeleteShaderPopUp(const char* path)
{
	if (ImGui::BeginPopupContextItem(path))
	{
		if (ImGui::Selectable("Delete Shader"))
		{
			shaderFile = path;

			showDeleteShaderConfirmationPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::CreateShaderConfirmationPopUp()
{
	if (ImGui::BeginPopupModal("Create Shader", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s", shaderFile.data());

		ImGui::PushItemWidth(200.0f);
		ImGui::InputText("##shaderName", shaderName, INPUT_BUF_SIZE);

		switch (shaderType)
		{
		case ShaderTypes::VertexShaderType:
			ImGui::Text(EXTENSION_VERTEX_SHADER_OBJECT);
			break;
		case ShaderTypes::FragmentShaderType:
			ImGui::Text(EXTENSION_FRAGMENT_SHADER_OBJECT);
			break;
		}

		if (ImGui::Button("Create", ImVec2(120.0f, 0)))
		{
			shaderFile.append(shaderName);

			ResourceData data;
			ResourceShaderObjectData shaderObjectData;
	
			switch (shaderType)
			{
			case ShaderTypes::VertexShaderType:
				shaderObjectData.SetSource(vShaderTemplate, strlen(vShaderTemplate));
				shaderFile.append(EXTENSION_VERTEX_SHADER_OBJECT);
				break;
			case ShaderTypes::FragmentShaderType:
				shaderObjectData.SetSource(fShaderTemplate, strlen(fShaderTemplate));
				shaderFile.append(EXTENSION_FRAGMENT_SHADER_OBJECT);
				break;
			}

			data.file = shaderFile;
			shaderObjectData.shaderType = shaderType;

			std::string outputFile;
			App->res->ExportFile(ResourceTypes::ShaderObjectResource, data, &shaderObjectData, outputFile, true);

			showCreateShaderConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
		{
			showCreateShaderConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::DeleteShaderConfirmationPopUp()
{
	if (ImGui::BeginPopupModal("Delete Shader", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string extension;
		App->fs->GetExtension(shaderFile.data(), extension);

		if (strcmp(extension.data(), EXTENSION_VERTEX_SHADER_OBJECT) == 0
			|| strcmp(extension.data(), EXTENSION_FRAGMENT_SHADER_OBJECT) == 0)
			ImGui::Text("Are you sure that you want to delete the following shader object?");
		else if (strcmp(extension.data(), EXTENSION_SHADER_PROGRAM) == 0)
			ImGui::Text("Are you sure that you want to delete the following shader program?");
		ImGui::TextColored(BLUE, "%s", shaderFile.data());

		if (ImGui::Button("Delete", ImVec2(120.0f, 0)))
		{
			App->fs->DeleteFileOrDir(shaderFile.data());

			showDeleteShaderConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
		{
			showDeleteShaderConfirmationPopUp = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

#endif // GAME
