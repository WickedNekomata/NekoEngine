#include "PanelAssets.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "ShaderImporter.h"

#include "imgui\imgui.h"
#include "Brofiler\Brofiler.h"

#include "Resource.h"

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

		bool treeNodeOpened = ImGui::TreeNodeEx(DIR_ASSETS);
		ChooseShaderPopUp(DIR_ASSETS);
		if (treeNodeOpened)
		{
			RecursiveDrawAssetsDir(App->fs->GetRootAssetsFile());
			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (showCreateShaderPopUp)
	{
		ImGui::OpenPopup("Create Shader");
		CreateShaderPopUp();
	}

	return true;
}

void PanelAssets::RecursiveDrawAssetsDir(AssetsFile* assetsFile)
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	assert(assetsFile != nullptr);

	ImGuiTreeNodeFlags treeNodeFlags;

	char id[DEFAULT_BUF_SIZE];

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

			sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", child->name.data(), child->path.data());
			if (ImGui::TreeNodeEx(id, treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
				SELECT(NULL);

			ChooseShaderPopUp(child->path.data());			

			if (treeNodeOpened)
			{
				if (!child->children.empty())
					RecursiveDrawAssetsDir(child);
				ImGui::TreePop();
			}
		}
		else
		{
			ResourceType type = ModuleResourceManager::GetResourceTypeByExtension(extension.data());
			
			treeNodeFlags = 0;

			if (type != ResourceType::MeshResource)
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			else
				treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

			switch (type)
			{
			case ResourceType::MeshResource:
				if (App->scene->selectedObject == ((MeshImportSettings*)child->importSettings))
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
				break;
			case ResourceType::TextureResource:
				if (App->scene->selectedObject == ((TextureImportSettings*)child->importSettings))
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
				break;
			case ResourceType::ShaderObjectResource:
			case ResourceType::ShaderProgramResource:
				if (App->scene->selectedObject == child->resource)
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
				break;
			}

			sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%s", child->name.data(), child->path.data());
			if (ImGui::TreeNodeEx(id, treeNodeFlags))
				treeNodeOpened = true;

			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
				&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
			{
				switch (type)
				{
				case ResourceType::MeshResource:
					SELECT((MeshImportSettings*)child->importSettings);
					break;
				case ResourceType::TextureResource:
					SELECT((TextureImportSettings*)child->importSettings);
					break;
				case ResourceType::ShaderObjectResource:
				case ResourceType::ShaderProgramResource:
					SELECT(child->resource);
					break;
				case ResourceType::NoResourceType:
					if (IS_SCENE(extension.data()))
					{
						SELECT(CurrentSelection::SelectedType::scene);
					}
					else
						SELECT(NULL);
					break;
				}
			}

			if (treeNodeOpened)
			{
				switch (type)
				{
				case ResourceType::MeshResource:
					for (std::map<std::string, uint>::const_iterator it = child->UUIDs.begin(); it != child->UUIDs.end(); ++it)
					{
						sprintf_s(id, DEFAULT_BUF_SIZE, "%s##%u", it->first.data(), it->second);
						if (ImGui::TreeNodeEx(id, ImGuiTreeNodeFlags_Leaf))
						{
							if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None)
								&& (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
								SELECT(NULL);

							SetResourceDragAndDropSource(type, it->second);
							ImGui::TreePop();
						}
					}
					break;
				case ResourceType::TextureResource:
					SetResourceDragAndDropSource(type, child->UUIDs.begin()->second);
					break;
				case ResourceType::ShaderObjectResource:
				case ResourceType::ShaderProgramResource:
					SetResourceDragAndDropSource(type, 0, child->resource);
					break;
				case ResourceType::NoResourceType:
					if (IS_SCENE(extension.data()))
						SetResourceDragAndDropSource(type, 0, nullptr, child->path.data());
					break;
				}
				ImGui::TreePop();				
			}
		}
	}
}

void PanelAssets::SetResourceDragAndDropSource(ResourceType type, uint UUID, const Resource* resource, const char* file) const
{
	switch (type)
	{
	case ResourceType::MeshResource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::TextureResource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("MATERIAL_INSPECTOR_SELECTOR", &UUID, sizeof(uint));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::ShaderObjectResource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("SHADER_OBJECT", &resource, sizeof(Resource*));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::ShaderProgramResource:

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("SHADER_PROGRAM", &resource, sizeof(Resource*));
			ImGui::EndDragDropSource();
		}
		break;

	case ResourceType::NoResourceType:
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

void PanelAssets::ChooseShaderPopUp(const char* path)
{
	char id[DEFAULT_BUF_SIZE];
	strcpy_s(id, DEFAULT_BUF_SIZE, path);
	if (ImGui::BeginPopupContextItem(id))
	{
		if (ImGui::Selectable("Create Vertex Shader"))
		{
			shaderType = ShaderType::VertexShaderType;
			strcpy_s(shaderName, strlen("New Vertex Shader") + 1, "New Vertex Shader");
			shaderFile = path;
			shaderFile.append("/");

			showCreateShaderPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Selectable("Create Fragment Shader"))
		{
			shaderType = ShaderType::FragmentShaderType;
			strcpy_s(shaderName, strlen("New Fragment Shader") + 1, "New Fragment Shader");
			shaderFile = path;
			shaderFile.append("/");

			showCreateShaderPopUp = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelAssets::CreateShaderPopUp()
{
	if (ImGui::BeginPopupModal("Create Shader", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s", shaderFile.data());

		ImGui::PushItemWidth(200.0f);
		ImGui::InputText("##shaderName", shaderName, INPUT_BUF_SIZE);

		switch (shaderType)
		{
		case ShaderType::VertexShaderType:
			ImGui::Text(EXTENSION_VERTEX_SHADER_OBJECT);
			break;
		case ShaderType::FragmentShaderType:
			ImGui::Text(EXTENSION_FRAGMENT_SHADER_OBJECT);
			break;
		}

		if (ImGui::Button("Create", ImVec2(120.0f, 0)))
		{
			shaderFile.append(shaderName);

			switch (shaderType)
			{
			case ShaderType::VertexShaderType:
				shaderFile.append(EXTENSION_VERTEX_SHADER_OBJECT);
				break;
			case ShaderType::FragmentShaderType:
				shaderFile.append(EXTENSION_FRAGMENT_SHADER_OBJECT);
				break;
			}

			if (App->shaderImporter->CreateShaderObject(shaderFile))
				App->res->ImportFile(shaderFile.data());
			
			System_Event newEvent;
			newEvent.type = System_Event_Type::RefreshAssets;
			App->PushSystemEvent(newEvent);

			showCreateShaderPopUp = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0)))
		{
			showCreateShaderPopUp = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

#endif // GAME