#include "PanelShaderEditor.h"

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
		ImGui::PushItemWidth(150.0f); ImGui::InputText("##name", shaderProgramName, INPUT_BUF_SIZE);
		ImGui::Separator();

		char shaderObjectName[DEFAULT_BUF_SIZE];

		ImGui::Text("Vertex Shaders");
		for (std::list<ResourceShaderObject*>::iterator it = vertexShaders.begin(); it != vertexShaders.end();)
		{
			if (*it != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##v%i", (*it)->GetName(), std::distance(vertexShaders.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Vertex##v%i", std::distance(vertexShaders.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->shaderType == ShaderType::VertexShaderType)
						*it = payload_n;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (*it != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT SHADER OBJECT##v%i", std::distance(vertexShaders.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "X##v%i", std::distance(vertexShaders.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = vertexShaders.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##vertex"))
			vertexShaders.push_back(nullptr);

		ImGui::Text("Fragment Shaders");
		for (std::list<ResourceShaderObject*>::iterator it = fragmentShaders.begin(); it != fragmentShaders.end();)
		{
			if (*it != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##v%i", (*it)->GetName(), std::distance(fragmentShaders.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Fragment##v%i", std::distance(fragmentShaders.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->shaderType == ShaderType::FragmentShaderType)
						*it = payload_n;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (*it != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT SHADER OBJECT##v%i", std::distance(fragmentShaders.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "X##f%i", std::distance(fragmentShaders.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = fragmentShaders.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##Fragment"))
			fragmentShaders.push_back(nullptr);

		// Link
		if (ImGui::Button("Link and Save"))
		{
			std::list<ResourceShaderObject*> shaderObjects;
			GetShaderObjects(shaderObjects);

			// Existing shader program?
			if (shaderProgram != nullptr)
			{
				shaderProgram->SetName(&shaderProgramName[0]);
				shaderProgram->SetShaderObjects(shaderObjects);
				if (!shaderProgram->Link())
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

					// Update shader objects in the meta
					App->shaderImporter->SetShaderObjectsToMeta(metaFile, shaderObjects);

					// Update last modification time in the meta
					int lastModTime = App->fs->GetLastModificationTime(output.data());
					Importer::SetLastModificationTimeToMeta(metaFile, lastModTime);			

					System_Event newEvent;
					newEvent.type = System_Event_Type::RefreshAssets;
					App->PushSystemEvent(newEvent);
				}
			}
			else
			{
				// Create a new resource for the shader program
				shaderProgram = (ResourceShaderProgram*)App->res->CreateNewResource(ResourceType::ShaderProgramResource);

				shaderProgram->SetName(&shaderProgramName[0]);
				shaderProgram->SetShaderObjects(shaderObjects);
				if (!shaderProgram->Link())
					shaderProgram->isValid = false;

				std::string output;
				if (App->shaderImporter->SaveShaderProgram(shaderProgram, output))
				{
					// Generate a new meta
					App->shaderImporter->GenerateShaderProgramMeta(shaderProgram, output);

					System_Event newEvent;
					newEvent.type = System_Event_Type::RefreshAssets;
					App->PushSystemEvent(newEvent);
				}
			}
		}

		ImGui::SameLine();

		// Try to link
		if (ImGui::Button("Link"))
		{
			std::list<uint> shaderObjects;
			GetShaderObjects(shaderObjects);

			ResourceShaderProgram::DeleteShaderProgram(ResourceShaderProgram::Link(shaderObjects));
		}
	}
	ImGui::End();

	if (!enabled)
	{
		shaderProgram = nullptr;
		strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
		vertexShaders.clear();
		fragmentShaders.clear();
	}

	return true;
}

void PanelShaderEditor::OpenShaderInShaderEditor(ResourceShaderProgram* shaderProgram)
{
	assert(shaderProgram != nullptr);

	enabled = true;
	this->shaderProgram = shaderProgram;

	strcpy_s(shaderProgramName, strlen(shaderProgram->GetName()) + 1, shaderProgram->GetName());
	vertexShaders = shaderProgram->GetShaderObjects(ShaderType::VertexShaderType);
	fragmentShaders = shaderProgram->GetShaderObjects(ShaderType::FragmentShaderType);
}

void PanelShaderEditor::GetShaderObjects(std::list<ResourceShaderObject*>& shaderObjects) const
{
	for (std::list<ResourceShaderObject*>::const_iterator it = vertexShaders.begin(); it != vertexShaders.end(); ++it)
		shaderObjects.push_back(*it);

	for (std::list<ResourceShaderObject*>::const_iterator it = fragmentShaders.begin(); it != fragmentShaders.end(); ++it)
		shaderObjects.push_back(*it);
}

void PanelShaderEditor::GetShaderObjects(std::list<uint>& shaderObjects) const
{
	for (std::list<ResourceShaderObject*>::const_iterator it = vertexShaders.begin(); it != vertexShaders.end(); ++it)
		shaderObjects.push_back((*it)->shaderObject);

	for (std::list<ResourceShaderObject*>::const_iterator it = fragmentShaders.begin(); it != fragmentShaders.end(); ++it)
		shaderObjects.push_back((*it)->shaderObject);
}