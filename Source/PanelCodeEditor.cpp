#include "PanelCodeEditor.h"

#include "Application.h"
#include "ShaderImporter.h"
#include "ResourceShaderObject.h"

PanelCodeEditor::PanelCodeEditor(char* name) : Panel(name)
{
	editor;
	auto lang = TextEditor::LanguageDefinition::GLSLCustom();

	editor.SetLanguageDefinition(lang);
}

PanelCodeEditor::~PanelCodeEditor()
{
}

bool PanelCodeEditor::Draw()
{
	auto cpos = editor.GetCursorPosition();
	ImGui::Begin("Text Editor Demo", &enabled, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, editor.CanUndo()))
				editor.Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, editor.CanRedo()))
				editor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
				editor.Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, editor.HasSelection()))
				editor.Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, editor.HasSelection()))
				editor.Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, ImGui::GetClipboardText() != nullptr))
				editor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				editor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				editor.SetPalette(TextEditor::GetLightPalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::Button("Save"))
	{
		uint shaderCompiled = ResourceShaderObject::Compile(editor.GetText().data(), currentShader->shaderType);
		if (shaderCompiled > 0)
		{
			ResourceShaderObject::DeleteShaderObject(currentShader->shaderObject);
			std::string text = editor.GetText();
			currentShader->SetSource(text.data(), text.size());
			currentShader->shaderObject = shaderCompiled;
			
			// Search for the meta associated to the file
			char metaFile[DEFAULT_BUF_SIZE];
			strcpy_s(metaFile, strlen(currentShader->file.data()) + 1, currentShader->file.data()); // file
			strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension
			
			// Reimport Shader Object file
			System_Event newEvent;
			newEvent.fileEvent.metaFile = metaFile;
			newEvent.type = System_Event_Type::ReimportFile;
			App->PushSystemEvent(newEvent);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Compile"))
	{
		uint shaderCompiled = ResourceShaderObject::Compile(editor.GetText().data(), currentShader->shaderType);
		ResourceShaderObject::DeleteShaderObject(shaderCompiled);
	}

	switch (currentShader->shaderType)
	{
	case ShaderType::VertexShaderType:
		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit),
			"Vertex Shader",
			currentShader->GetName();
		break;
	case ShaderType::FragmentShaderType:
		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit),
			"Vertex Shader",
			currentShader->GetName();
		break;
	}

	editor.Render("TextEditor");
	ImGui::End();

	return true;
}

void PanelCodeEditor::OpenShaderInCodeEditor(ResourceShaderObject* shader)
{
	enabled = true;
	editor.SetText(shader->GetSource());
	currentShader = shader;
}