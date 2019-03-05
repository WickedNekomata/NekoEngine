#include "PanelCodeEditor.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"
#include "ResourceShaderObject.h"

// Allows the modification of an existing shader object

PanelCodeEditor::PanelCodeEditor(const char* name) : Panel(name)
{
	editor;
	auto lang = TextEditor::LanguageDefinition::GLSLCustom();

	editor.SetLanguageDefinition(lang);
}

PanelCodeEditor::~PanelCodeEditor() {}

bool PanelCodeEditor::Draw()
{
	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUuid);

	if (shaderObject != nullptr)
	{
		auto cpos = editor.GetCursorPosition();
		ImGui::Begin("Code Editor", &enabled, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_FirstUseEver);

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

				if (ImGui::MenuItem("Select All", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark Palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light Palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// Compile
		if (ImGui::Button("Compile and Save"))
		{
			TryCompile();

			// Update the existing shader object
			shaderObject->SetSource(editor.GetText().data(), editor.GetText().length());
			if (!shaderObject->Compile())
				shaderObject->isValid = false;
			else
				shaderObject->isValid = true;

			// Export the existing file
			std::string outputFile;
			App->res->ExportFile(ResourceTypes::ShaderObjectResource, shaderObject->GetData(), &shaderObject->GetSpecificData(), outputFile, true, false);
		}

		ImGui::SameLine();

		// Try to compile
		if (ImGui::Button("Compile"))
			TryCompile();

		ImGui::SameLine();

		if (ImGui::Button("Clean Errors"))
		{
			TextEditor::ErrorMarkers markers;
			editor.SetErrorMarkers(markers);
		}

		switch (shaderObject->GetShaderObjectType())
		{
		case ShaderObjectTypes::VertexType:
			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit,
				"Vertex Shader",
				shaderObject->GetName());
			break;
		case ShaderObjectTypes::FragmentType:
			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit,
				"Vertex Shader",
				shaderObject->GetName());
			break;
		case ShaderObjectTypes::GeometryType:
			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit,
				"Geometry Shader",
				shaderObject->GetName());
			break;
		}

		editor.Render("TextEditor");

		ImGui::End();
	}
	else
		enabled = false;

	if (!enabled)
	{
		shaderObjectUuid = 0;
		editor.Delete();
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

void PanelCodeEditor::OpenShaderInCodeEditor(uint shaderObjectUuid)
{
	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUuid);
	assert(shaderObject != nullptr);

	enabled = true;
	this->shaderObjectUuid = shaderObjectUuid;

	editor.SetText(shaderObject->GetSource());
}

void PanelCodeEditor::SetError(int line, const char* error)
{
	if (!enabled)
		return;

	TextEditor::ErrorMarkers markers;
	markers[line] = error;
	editor.SetErrorMarkers(markers);
}

// ----------------------------------------------------------------------------------------------------

bool PanelCodeEditor::TryCompile()
{
	TextEditor::ErrorMarkers markers;
	editor.SetErrorMarkers(markers);

	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUuid);
	uint tryCompile = ResourceShaderObject::Compile(editor.GetText().data(), shaderObject->GetShaderObjectType());

	if (tryCompile > 0)
	{
		ResourceShaderObject::DeleteShaderObject(tryCompile);
		return true;
	}

	return false;
}

#endif