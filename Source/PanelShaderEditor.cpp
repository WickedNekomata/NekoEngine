#include "PanelShaderEditor.h"

#include "Application.h"
#include "ModuleGui.h"
#include "PanelCodeEditor.h"

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

	for (auto it = vertexShaders.begin(); it != vertexShaders.end(); ++it)
	{
		std::string shader("%u", *it);
		ImGui::Button(shader.data(), ImVec2(150.0f, 0.0f));

		ImGui::SameLine();
		
		if (ImGui::Button("...##vertex"))
			App->gui->panelCodeEditor->OpeninCodeEditor(vShaderTemplate);

		ImGui::SameLine();

		if (ImGui::Button("X##vertex"))
			vertexShaders.remove(*it);
	}

	ImGui::SameLine();

	if (ImGui::Button("+##vertex"))
		vertexShaders.push_back(0);

	ImGui::Text("Fragment Shader");

	for (auto it = fragmentShaders.begin(); it != fragmentShaders.end(); ++it)
	{
		std::string shader("%u", *it);
		ImGui::Button(shader.data(), ImVec2(150.0f, 0.0f));

		ImGui::SameLine();

		if (ImGui::Button("...##fragment")) {
			App->gui->panelCodeEditor->OpeninCodeEditor(fShaderTemplate);
		}

		ImGui::SameLine();

		if (ImGui::Button("X##fragment"))
			fragmentShaders.remove(*it);
	}

	ImGui::SameLine();

	if (ImGui::Button("+##Fragment"))
		fragmentShaders.push_back(0);

	if (ImGui::Button("Compile and Save")) {

	}

	ImGui::End();

	return true;
}
