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

PanelShaderEditor::PanelShaderEditor(char* name) : Panel(name)
{
	char* defaultName = "Default Name";
	memcpy(shaderProgramName, defaultName, strlen(defaultName));
}

PanelShaderEditor::~PanelShaderEditor()
{
}

bool PanelShaderEditor::Draw()
{
	ImGui::Begin(name, &enabled);

	ImGui::Text("Shader Program:"); ImGui::SameLine();

	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	ImGui::PushItemWidth(100.0f);
	ImGui::InputText("##ShaderName", shaderProgramName, IM_ARRAYSIZE(shaderProgramName), inputFlag);

	ImGui::Separator();

	ImGui::Text("Vertex Shader");

	char itemName[DEFAULT_BUF_SIZE];

	for (auto it = vertexShaders.begin(); it != vertexShaders.end();)
	{
		if (*it != nullptr) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##v%i", (*it)->GetName(), std::distance(vertexShaders.begin(), it));
			ImGui::Button(itemName, ImVec2(150.0f, 0.0f));
		}
		else {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Empty##v%i", std::distance(vertexShaders.begin(), it));
			ImGui::Button(itemName, ImVec2(150.0f, 0.0f));
		}

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

		if (*it != nullptr) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "...##v%i", std::distance(vertexShaders.begin(), it));
			if (ImGui::Button(itemName))
				App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);

			ImGui::SameLine();
		}

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "X##v%i", std::distance(vertexShaders.begin(), it));
		if (ImGui::Button(itemName))
		{
			it = vertexShaders.erase(it);
			continue;
		}

		it++;
	}

	if (ImGui::Button("+##vertex"))
		vertexShaders.push_back(nullptr);

	ImGui::Text("Fragment Shader");

	for (auto it = fragmentShaders.begin(); it != fragmentShaders.end();)
	{
		if (*it != nullptr) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##f%i", (*it)->GetName(), std::distance(fragmentShaders.begin(), it));
			ImGui::Button(itemName, ImVec2(150.0f, 0.0f));
		}
		else {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Empty##f%i", std::distance(fragmentShaders.begin(), it));
			ImGui::Button(itemName, ImVec2(150.0f, 0.0f));
		}

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

		if (*it != nullptr) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "...##f%i", std::distance(fragmentShaders.begin(), it));
			if (ImGui::Button(itemName)) {
				App->gui->panelCodeEditor->OpenShaderInCodeEditor(*it);
			}

			ImGui::SameLine();
		}

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "X##f%i", std::distance(fragmentShaders.begin(), it));
		if (ImGui::Button(itemName))
		{
			it = fragmentShaders.erase(it);
			continue;
		}

		it++;
	}

	if (ImGui::Button("+##Fragment"))
		fragmentShaders.push_back(nullptr);

	if (ImGui::Button("Link and Save")) {

		std::list<ResourceShaderObject*> readyToLink;

		for (auto it = vertexShaders.begin(); it != vertexShaders.end(); ++it)
			readyToLink.push_back(*it);

		for (auto it = fragmentShaders.begin(); it != fragmentShaders.end(); ++it)
			readyToLink.push_back(*it);

		GLuint shaderProgram = ResourceShaderProgram::Link(readyToLink);
		if (shaderProgram > 0)
		{
			// TODO: mix this together with the Resource Manager (handle specific cases)
			std::string outputFile;
			if (App->shaderImporter->SaveShaderProgram(&shaderProgramName, shaderProgram, outputFile))
			{
				ResourceShaderProgram* resource = (ResourceShaderProgram*)App->res->CreateNewResource(ResourceType::ShaderProgramResource);
				resource->shaderProgram = shaderProgram;
				resource->shaderObjects = readyToLink;

				std::string outputMetaFile;
				App->shaderImporter->GenerateShaderProgramMeta(resource, outputMetaFile);

				System_Event newEvent;
				newEvent.type = System_Event_Type::RefreshAssets;
				App->PushSystemEvent(newEvent);
			}
		}
	}

	ImGui::End();

	if (!enabled)
	{
		vertexShaders.clear();
		fragmentShaders.clear();
		memset(shaderProgramName, ' ', strlen(shaderProgramName));
		App->gui->panelCodeEditor->SetOnOff(false);
	}

	return true;
}

void PanelShaderEditor::OpenFromShaderProgram(const ResourceShaderProgram* program)
{
	enabled = true;

	for (auto it = program->shaderObjects.begin(); it != program->shaderObjects.end(); ++it)
	{
		//if ((*it)) TODO: get type of shaderObject D:
	}

	memcpy(shaderProgramName, program->GetName(), strlen(program->GetName()));

}
