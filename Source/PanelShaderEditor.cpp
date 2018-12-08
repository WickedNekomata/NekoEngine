#include "PanelShaderEditor.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGui.h"
#include "PanelCodeEditor.h"
#include "ResourceShaderObject.h"

#include "Globals.h"
#include "imgui/imgui.h"

PanelShaderEditor::PanelShaderEditor(char* name) : Panel(name)
{
}

PanelShaderEditor::~PanelShaderEditor()
{
}

bool PanelShaderEditor::Draw()
{
	ImGui::Begin(name, &enabled);

	ImGui::Text("Name:"); 
	ImGui::SameLine();
	static char shaderProgramName[INPUT_BUF_SIZE];
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

	if (ImGui::Button("Compile and Save")) {

	}

	ImGui::End();

	return true;
}
