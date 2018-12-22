#include "PanelCodeEditor.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ShaderImporter.h"
#include "ResourceShaderObject.h"

// Allows the modification of an existing shader object

PanelCodeEditor::PanelCodeEditor(char* name) : Panel(name)
{
	editor;
	auto lang = TextEditor::LanguageDefinition::GLSLCustom();

	editor.SetLanguageDefinition(lang);
}

PanelCodeEditor::~PanelCodeEditor() {}

bool PanelCodeEditor::Draw()
{
	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUUID);

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
			// Update the parameters of the shader object
			shaderObject->SetSource(editor.GetText().data(), editor.GetText().length());
			if (!shaderObject->Compile())
				shaderObject->isValid = false;
			else
				shaderObject->isValid = true;

			std::string output;
			if (App->shaderImporter->SaveShaderObject(shaderObject, output, true))
			{
				// Search for the meta associated to the file
				char metaFile[DEFAULT_BUF_SIZE];
				strcpy_s(metaFile, strlen(output.data()) + 1, output.data()); // file
				strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

				// Update last modification time in the meta
				int lastModTime = App->fs->GetLastModificationTime(output.data());
				Importer::SetLastModificationTimeToMeta(metaFile, lastModTime);

				App->fs->AddMeta(metaFile, lastModTime);

				System_Event newEvent;
				newEvent.type = System_Event_Type::RefreshFiles;
				App->PushSystemEvent(newEvent);
			}
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

		switch (shaderObject->shaderType)
		{
		case ShaderType::VertexShaderType:
			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit,
				"Vertex Shader",
				shaderObject->GetName());
			break;
		case ShaderType::FragmentShaderType:
			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit,
				"Vertex Shader",
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
		shaderObjectUUID = 0;
		editor.Delete();
	}

	return true;
}

void PanelCodeEditor::OpenShaderInCodeEditor(uint shaderObjectUUID)
{
	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUUID);
	assert(shaderObject != nullptr);

	enabled = true;
	this->shaderObjectUUID = shaderObjectUUID;

	editor.SetText(shaderObject->GetSource());
}

uint PanelCodeEditor::GetShaderObjectUUID() const
{
	return shaderObjectUUID;
}

void PanelCodeEditor::SetError(int line, const char* error)
{
	if (!enabled)
		return;

	TextEditor::ErrorMarkers markers;
	markers[line] = error;
	editor.SetErrorMarkers(markers);
}

bool PanelCodeEditor::TryCompile()
{
	bool ret = false;

	TextEditor::ErrorMarkers markers;
	editor.SetErrorMarkers(markers);

	ResourceShaderObject* shaderObject = (ResourceShaderObject*)App->res->GetResource(shaderObjectUUID);
	uint tryCompile = ResourceShaderObject::Compile(editor.GetText().data(), shaderObject->shaderType);

	if (tryCompile > 0)
		ret = true;

	ResourceShaderObject::DeleteShaderObject(tryCompile);

	return ret;
}

#endif