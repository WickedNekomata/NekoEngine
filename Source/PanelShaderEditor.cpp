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
	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUuid);
	
	if ((shaderProgram != nullptr && shaderProgramUuid > 0)
		|| (shaderProgram == nullptr && shaderProgramUuid == 0))
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
			for (std::vector<uint>::iterator it = vertexShadersUuids.begin(); it != vertexShadersUuids.end();)
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
						if (payload_n->GetShaderObjectType() == ShaderObjectTypes::VertexType)
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

			ImGui::Text("Geometry Shaders");
			for (std::vector<uint>::iterator it = geometryShadersUuids.begin(); it != geometryShadersUuids.end();)
			{
				ResourceShaderObject* geometryShader = (ResourceShaderObject*)App->res->GetResource(*it);

				if (geometryShader != nullptr)
					sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "%s##g%i", geometryShader->GetName(), std::distance(geometryShadersUuids.begin(), it));
				else
					sprintf_s(shaderObjectName, INPUT_BUF_SIZE, "Empty Geometry##g%i", std::distance(geometryShadersUuids.begin(), it));
				ImGui::Button(shaderObjectName, ImVec2(150.0f, 0.0f));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_OBJECT"))
					{
						ResourceShaderObject* payload_n = *(ResourceShaderObject**)(payload->Data);
						if (payload_n->GetShaderObjectType() == ShaderObjectTypes::GeometryType)
							*it = payload_n->GetUuid();
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();

				if (geometryShader != nullptr)
				{
					sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "EDIT##g%i", std::distance(geometryShadersUuids.begin(), it));
					if (ImGui::Button(shaderObjectName))
						App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

					ImGui::SameLine();
				}

				sprintf_s(shaderObjectName, DEFAULT_BUF_SIZE, "-##g%i", std::distance(geometryShadersUuids.begin(), it));
				if (ImGui::Button(shaderObjectName))
				{
					it = geometryShadersUuids.erase(it);
					continue;
				}

				++it;
			}

			if (ImGui::Button("+##geometry"))
				geometryShadersUuids.push_back(0);

			ImGui::Text("Fragment Shaders");
			for (std::vector<uint>::iterator it = fragmentShadersUuids.begin(); it != fragmentShadersUuids.end();)
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
						if (payload_n->GetShaderObjectType() == ShaderObjectTypes::FragmentType)
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

			if (ImGui::Button("+##fragment"))
				fragmentShadersUuids.push_back(0);

			// Link
			if (ImGui::Button("Link and Save"))
			{
				if (TryLink())
				{
					std::vector<uint> shaderObjectsUuids;
					if (GetShaderObjects(shaderObjectsUuids) > 0)
					{
						std::string outputFile;
						if (shaderProgram != nullptr)
						{
							// Update the existing shader program
							shaderProgram->SetName(shaderProgramName);
							shaderProgram->SetShaderProgramType(ShaderProgramTypes::Custom);
							shaderProgram->SetShaderObjects(shaderObjectsUuids);

							// Export the existing file
							App->res->ExportFile(ResourceTypes::ShaderProgramResource, shaderProgram->GetData(), &shaderProgram->GetSpecificData(), outputFile, true, false);
							
							shaderProgram->Link();
						}
						else
						{
							// Basic shader program
							ResourceData data;
							ResourceShaderProgramData shaderProgramData;
							
							data.name = shaderProgramName;
							shaderProgramData.shaderProgramType = ShaderProgramTypes::Custom;
							shaderProgramData.shaderObjectsUuids = shaderObjectsUuids;

							// Export the new file
							shaderProgram = (ResourceShaderProgram*)App->res->ExportFile(ResourceTypes::ShaderProgramResource, data, &shaderProgramData, outputFile);
							
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
				geometryShadersUuids.clear();
				fragmentShadersUuids.clear();
			}
		}
		ImGui::End();
	}
	else
		enabled = false;

	if (!enabled)
	{
		shaderProgramUuid = 0;
		strcpy_s(shaderProgramName, strlen("New Shader Program") + 1, "New Shader Program");
		vertexShadersUuids.clear();
		geometryShadersUuids.clear();
		fragmentShadersUuids.clear();
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

void PanelShaderEditor::OpenShaderInShaderEditor(uint shaderProgramUuid)
{
	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUuid);
	assert(shaderProgram != nullptr);

	enabled = true;
	this->shaderProgramUuid = shaderProgramUuid;

	strcpy_s(shaderProgramName, strlen(shaderProgram->GetName()) + 1, shaderProgram->GetName());

	vertexShadersUuids.clear();
	shaderProgram->GetShaderObjects(vertexShadersUuids, ShaderObjectTypes::VertexType);
	geometryShadersUuids.clear();
	shaderProgram->GetShaderObjects(geometryShadersUuids, ShaderObjectTypes::GeometryType);
	fragmentShadersUuids.clear();
	shaderProgram->GetShaderObjects(fragmentShadersUuids, ShaderObjectTypes::FragmentType);
}

// ----------------------------------------------------------------------------------------------------

bool PanelShaderEditor::TryLink() const
{
	std::vector<uint> shaderObjectsUuids;
	if (GetShaderObjects(shaderObjectsUuids) > 0)
	{
		uint tryLink = ResourceShaderProgram::Link(shaderObjectsUuids);

		if (tryLink > 0)
		{
			ResourceShaderProgram::DeleteShaderProgram(tryLink);
			return true;
		}
	}
	else
		CONSOLE_LOG(LogTypes::Error, "Shader Program could not be linked since one or more shader objects are null or don't exist");

	return false;
}

uint PanelShaderEditor::GetShaderObjects(std::vector<uint>& shaderObjectsUuids) const
{
	for (uint i = 0; i < vertexShadersUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(vertexShadersUuids[i]);
		if (shaderObject != nullptr && shaderObject->isValid)
			shaderObjectsUuids.push_back(vertexShadersUuids[i]);
	}

	for (uint i = 0; i < geometryShadersUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(geometryShadersUuids[i]);
		if (shaderObject != nullptr && shaderObject->isValid)
			shaderObjectsUuids.push_back(geometryShadersUuids[i]);
	}

	for (uint i = 0; i < fragmentShadersUuids.size(); ++i)
	{
		ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(fragmentShadersUuids[i]);
		if (shaderObject != nullptr && shaderObject->isValid)
			shaderObjectsUuids.push_back(fragmentShadersUuids[i]);
	}

	return shaderObjectsUuids.size();
}

#endif