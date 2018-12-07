#include "PanelCodeEditor.h"

#include "Application.h"
#include "ShaderImporter.h"

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
			auto textToSave = editor.GetText();
			/// save shader object
		}

		ImGui::SameLine();

		if (ImGui::Button("Compile"))
		{
			// Compile Shader Object
		}

	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
		editor.IsOverwrite() ? "Ovr" : "Ins",
		editor.CanUndo() ? "*" : " ",
		editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

	editor.Render("TextEditor");
	ImGui::End();
	return true;
}

void PanelCodeEditor::OpeninCodeEditor(const char* buffer)
{
	enabled = true;
	editor.SetText(buffer);
}