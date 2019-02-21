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

PanelShaderEditor::PanelShaderEditor(const char* name) : Panel(name)
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

		ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUuid);
		if (shaderProgram != nullptr)
			ImGui::TextColored(BLUE, "%s", shaderProgram->GetFile());

		ImGui::Separator();

		char shaderObjectName[DEFAULT_BUF_SIZE];

		ImGui::Text("Vertex Shaders");
		for (std::list<uint>::iterator it = vertexShadersUuids.begin(); it != vertexShadersUuids.end();)
		{
			ResourceShaderObject* vertexShader = (ResourceShaderObject*)App->res->GetResource(*it);

			if (vertexShader != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##v%i", vertexShader->GetName(), std::distance(vertexShadersUuids.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Vertex##v%i", std::distance(vertexShadersUuids.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->GetShaderType() == ShaderTypes::VertexShaderType)
						*it = payload_n->GetUuid();
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (vertexShader != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##v%i", std::distance(vertexShadersUuids.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "-##v%i", std::distance(vertexShadersUuids.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = vertexShadersUuids.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##vertex"))
			vertexShadersUuids.push_back(0);

		ImGui::Text("Fragment Shaders");
		for (std::list<uint>::iterator it = fragmentShadersUuids.begin(); it != fragmentShadersUuids.end();)
		{
			ResourceShaderObject* fragmentShader = (ResourceShaderObject*)App->res->GetResource(*it);

			if (fragmentShader != nullptr)
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##f%i", fragmentShader->GetName(), std::distance(fragmentShadersUuids.begin(), it));
			else
				sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Fragment##f%i", std::distance(fragmentShadersUuids.begin(), it));
			ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
				{
					ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
					if (payload_n->GetShaderType() == ShaderTypes::FragmentShaderType)
						*it = payload_n->GetUuid();
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();

			if (fragmentShader != nullptr)
			{
				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##f%i", std::distance(fragmentShadersUuids.begin(), it));
				if (ImGui::Button(shaderObjectName))
					App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

				ImGui::SameLine();
			}

			sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "-##f%i", std::distance(fragmentShadersUuids.begin(), it));
			if (ImGui::Button(shaderObjectName))
			{
				it = fragmentShadersUuids.erase(it);
				continue;
			}

			++it;
		}

		if (ImGui::Button("+##Fragment"))
			fragmentShadersUuids.push_back(0);

		// Link
		if (ImGui::Button("Link and Save"))
		{
			if (TryLink())
			{
				std::list<ResourceShaderObject*> shaderObjects;
				if (GetShaderObjects(shaderObjects))
				{			
					if (shaderProgram != nullptr)
					{
						// Update the existing shader program
						shaderProgram->SetName(name);
						shaderProgram->SetShaderObjects(shaderObjects);
						shaderProgram->Link();

						App->res->ExportFile(ResourceTypes::ShaderProgramResource, shaderProgram->GetData(), &shaderProgram->GetSpecificData(), true);
					}
					else
					{
						// Create a new file and resource for the shader program
						ResourceData data;
						ResourceShaderProgramData shaderProgramData;
						data.name = shaderProgramName;
						shaderProgramData.shaderObjects = shaderObjects;

						shaderProgram = (ResourceShaderProgram*)App->res->ExportFile(ResourceTypes::ShaderProgramResource, data, &shaderProgramData);
						shaderProgramUuid = shaderProgram->GetUuid();
					}
				}
				else
					CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked since one or more shader objects are null or don't exist");
			}
		}

		ImGui::SameLine();

		// Try to link
		if (ImGui::Button("Link"))
			TryLink();

		ImGui::SameLine();

		if (ImGui::Button("New Shader"))
		{
			shaderProgramUuid = 0;
			strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
			vertexShadersUuids.clear();
			fragmentShadersUuids.clear();
		}
	}
	ImGui::End();

	if (!enabled)
	{
		shaderProgramUuid = 0;
		strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
		vertexShadersUuids.clear();
		fragmentShadersUuids.clear();
	}

	return true;
}

void PanelShaderEditor::OpenShaderInShaderEditor(uint shaderProgramUuid)
{
	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUuid);
	assert(shaderProgram != nullptr);

	enabled = true;
	this->shaderProgramUuid = shaderProgramUuid;

	strcpy_s(shaderProgramName, strlen(shaderProgram->GetName()) + 1, shaderProgram->GetName());

	vertexShadersUuids.clear();
	std::list<ResourceShaderObject*> shaderObjects = shaderProgram->GetShaderObjects(ShaderTypes::VertexShaderType);
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		vertexShadersUuids.push_back((*it)->GetUuid());

	fragmentShadersUuids.clear();
	shaderObjects = shaderProgram->GetShaderObjects(ShaderTypes::FragmentShaderType);
	for (std::list<ResourceShaderObject*>::const_iterator it = shaderObjects.begin(); it != shaderObjects.end(); ++it)
		fragmentShadersUuids.push_back((*it)->GetUuid());
}

uint PanelShaderEditor::GetShaderProgramUuid() const
{
	return shaderProgramUuid;
}

bool PanelShaderEditor::GetShaderObjects(std::list<ResourceShaderObject*>& shaderObjects) const
{
	if (vertexShadersUuids.empty())
		return false;
	for (std::list<uint>::const_iterator it = vertexShadersUuids.begin(); it != vertexShadersUuids.end(); ++it)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(*it);
		if (shaderObject == nullptr || shaderObject->shaderObject == 0)
			return false;
		shaderObjects.push_back(shaderObject);
	}

	if (fragmentShadersUuids.empty())
		return false;
	for (std::list<uint>::const_iterator it = fragmentShadersUuids.begin(); it != fragmentShadersUuids.end(); ++it)
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

bool PanelShaderEditor::TryLink() const
{
	bool ret = false;

	std::list<ResourceShaderObject*> shaderObjects;
	if (GetShaderObjects(shaderObjects))
	{
		uint tryLink = ResourceShaderProgram::Link(shaderObjects);

		if (tryLink > 0)
			ret = true;

		ResourceShaderProgram::DeleteShaderProgram(tryLink);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked since one or more shader objects are null or don't exist");

	return ret;
}

#endif