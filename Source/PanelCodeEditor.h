#ifndef __PANEL_CODE_EDITOR_H__
#define __PANEL_CODE_EDITOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "Globals.h"

#include "ImGui\imgui.h"
#include "ImGuiColorTextEdit\TextEditor.h"

class ResourceShaderObject;

class PanelCodeEditor : public Panel
{
public:

	PanelCodeEditor(char* name);
	~PanelCodeEditor();

	bool Draw();

	void OpenShaderInCodeEditor(uint shaderObjectUUID);
	uint GetShaderObjectUUID() const;

	void SetError(int line, const char* error);

private:

	bool TryCompile() const;

private:

	uint shaderObjectUUID = 0;

	const char* fileToEdit = "";
	TextEditor editor;
};

#endif

#endif // GAME
