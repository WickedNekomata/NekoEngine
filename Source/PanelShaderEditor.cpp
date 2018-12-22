#include "PanelShaderEditor.h"

#ifndef GAMEMODE

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleGui.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"
#include "PanelCodeEditor.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

#include "imgui/imgui.h"

// Allows the modification of an existing shader program or the creation of a new shader program

PanelShaderEditor::PanelShaderEditor(char* name) : Panel(name) 
{
	strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
}

PanelShaderEditor::~PanelShaderEditor() {}

bool PanelShaderEditor::Draw()
{
	if (ImGui::Begin(name, &enabled))
	{
		ImGui::Text("Shader Program:"); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f); 
		ImGui::InputText("##name", shaderProgramName, INPUT_BUF_SIZE);
		ImGui::Separator();

		char shaderObjectName[DEFAULT_BUF_SIZE];

		ImGui::Text("Vertex Shaders");
		for (std::list<uint>::iterator it = vertexShadersUUIDs.begin(); it != vertexShadersUUIDs.end();)
		{
			ResourceShaderObject* vertexShader = (ResourceShaderObject*)App->res->GetResource(*it);

			if (vertexShader != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##v%i", vertexShader->GetName(), std::distance(vertexShadersUUIDs.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Vertex##v%i", std::distance(vertexShadersUUIDs.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->shaderType == ShaderType::VertexShaderType)
						*it = payload_n->GetUUID();
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (vertexShader != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##v%i", std::distance(vertexShadersUUIDs.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "-##v%i", std::distance(vertexShadersUUIDs.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = vertexShadersUUIDs.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##vertex"))
			vertexShadersUUIDs.push_back(0);

		ImGui::Text("Fragment Shaders");
		for (std::list<uint>::iterator it = fragmentShadersUUIDs.begin(); it != fragmentShadersUUIDs.end();)
		{
			ResourceShaderObject* fragmentShader = (ResourceShaderObject*)App->res->GetResource(*it);

			if (fragmentShader != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##f%i", fragmentShader->GetName(), std::distance(fragmentShadersUUIDs.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Fragment##f%i", std::distance(fragmentShadersUUIDs.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->shaderType == ShaderType::FragmentShaderType)
						*it = payload_n->GetUUID();
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (fragmentShader != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##f%i", std::distance(fragmentShadersUUIDs.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "-##f%i", std::distance(fragmentShadersUUIDs.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = fragmentShadersUUIDs.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##Fragment"))
			fragmentShadersUUIDs.push_back(0);

		// Link
		if (ImGui::Button("Link and Save"))
		{
			if (TryLink())
			{
				std::list<ResourceShaderObject*> shaderObjects;
				if (GetShaderObjects(shaderObjects))
				{
					ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUUID);

					// Existing shader program
					if (shaderProgram != nullptr)
					{
						// Update the parameters of the shader program
						shaderProgram->SetName(shaderProgramName);
						shaderProgram->SetShaderObjects(shaderObjects);
						if (!shaderProgram->Link(false))
							shaderProgram->isValid = false;
						else
							shaderProgram->isValid = true;

						std::string output;
						if (App->shaderImporter->SaveShaderProgram(shaderProgram, output, true))
						{
							// Search for the meta associated to the file
							char metaFile[DEFAULT_BUF_SIZE];
							strcpy_s(metaFile, strlen(output.data()) + 1, output.data()); // file
							strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

							// Update name in the meta
							App->shaderImporter->SetShaderNameToMeta(metaFile, shaderProgram->GetName());

							// Update shader objects in the meta
							App->shaderImporter->SetShaderObjectsToMeta(metaFile, shaderObjects);

							// Update last modification time in the meta
							int lastModTime = App->fs->GetLastModificationTime(output.data());
							Importer::SetLastModificationTimeToMeta(metaFile, lastModTime);

							App->fs->AddMeta(metaFile, lastModTime);

							System_Event newEvent;
							newEvent.type = System_Event_Type::RefreshFiles;
							App->PushSystemEvent(newEvent);
						}
					}
					else
					{
						// Create a new resource for the shader program
						shaderProgram = (ResourceShaderProgram*)App->res->CreateNewResource(ResourceType::ShaderProgramResource);
						shaderProgramUUID = shaderProgram->GetUUID();

						// Update the parameters of the shader program
						shaderProgram->SetName(shaderProgramName);
						shaderProgram->SetShaderObjects(shaderObjects);
						if (!shaderProgram->Link(false))
							shaderProgram->isValid = false;
						else
							shaderProgram->isValid = true;

						std::string output;
						if (App->shaderImporter->SaveShaderProgram(shaderProgram, output))
						{
							shaderProgram->file = shaderProgram->exportedFile = output;

							// Generate a new meta
							output.clear();
							App->shaderImporter->GenerateShaderProgramMeta(shaderProgram, output);
							int lastModTime = App->fs->GetLastModificationTime(output.data());
							App->fs->AddMeta(output.data(), lastModTime);

							System_Event newEvent;
							newEvent.type = System_Event_Type::RefreshFiles;
							App->PushSystemEvent(newEvent);
						}
					}
				}
				else
					CONSOLE_LOG("Shader Program could not be linked since one or more shader objects are null or don't exist");
			}
		}

		ImGui::SameLine();

		// Try to link
		if (ImGui::Button("Link"))
			TryLink();
	}
	ImGui::End();

	if (!enabled)
	{
		shaderProgramUUID = 0;
		strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
		vertexShadersUUIDs.clear();
		fragmentShadersUUIDs.clear();
	}

	return true;
}

void PanelShaderEditor::OpenShaderInShaderEditor(uint shaderProgramUUID)
{
	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUUID);
	assert(shaderProgram != nullptr);

	enabled = true;
	this->shaderProgramUUID = shaderProgramUUID;

	strcpy_s(shaderProgramName, strlen(shaderProgram->GetName()) + 1, shaderProgram->GetName());

	vertexShadersUUIDs.clear();
	std::list<ResourceShaderObject*> shaderObjects = shaderProgram->GetShaderObjects(ShaderType::VertexShaderType);
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		vertexShadersUUIDs.push_back((*it)->GetUUID());

	fragmentShadersUUIDs.clear();
	shaderObjects = shaderProgram->GetShaderObjects(ShaderType::FragmentShaderType);
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		fragmentShadersUUIDs.push_back((*it)->GetUUID());
}

uint PanelShaderEditor::GetShaderProgramUUID() const
{
	return shaderProgramUUID;
}

bool PanelShaderEditor::GetShaderObjects(std::list<ResourceShaderObject*>& shaderObjects) const
{
	if (vertexShadersUUIDs.empty())
		return false;
	for (std::list<uint>::const_iterator it = vertexShadersUUIDs.begin(); it != vertexShadersUUIDs.end(); ++it)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(*it);
		if (shaderObject == nullptr || shaderObject->shaderObject == 0)
			return false;
		shaderObjects.push_back(shaderObject);
	}

	if (fragmentShadersUUIDs.empty())
		return false;
	for (std::list<uint>::const_iterator it = fragmentShadersUUIDs.begin(); it != fragmentShadersUUIDs.end(); ++it)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(*it);
		if (shaderObject == nullptr || shaderObject->shaderObject == 0)
			return false;
		shaderObjects.push_back(shaderObject);
	}

	if (shaderObjects.empty())
		return false;

	return true;
}

bool PanelShaderEditor::GetShaderObjectsIDs(std::list<uint>& shaderObjectsIDs) const
{
	if (vertexShadersUUIDs.empty())
		return false;
	for (std::list<uint>::const_iterator it = vertexShadersUUIDs.begin(); it != vertexShadersUUIDs.end(); ++it)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(*it);
		if (shaderObject == nullptr || shaderObject->shaderObject == 0)
			return false;
		shaderObjectsIDs.push_back(shaderObject->shaderObject);
	}		

	if (fragmentShadersUUIDs.empty())
		return false;
	for (std::list<uint>::const_iterator it = fragmentShadersUUIDs.begin(); it != fragmentShadersUUIDs.end(); ++it)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(*it);
		if (shaderObject == nullptr || shaderObject->shaderObject == 0)
			return false;
		shaderObjectsIDs.push_back(shaderObject->shaderObject);
	}

	if (shaderObjectsIDs.empty())
		return false;

	return true;
}

bool PanelShaderEditor::TryLink() const
{
	bool ret = false;

	std::list<uint> shaderObjectsIDs;
	if (GetShaderObjectsIDs(shaderObjectsIDs))
	{
		uint tryLink = ResourceShaderProgram::Link(shaderObjectsIDs);

		if (tryLink > 0)
			ret = true;

		ResourceShaderProgram::DeleteShaderProgram(tryLink);
	}
	else
		CONSOLE_LOG("Shader Program could not be linked since one or more shader objects are null or don't exist");

	return ret;
}

#endif