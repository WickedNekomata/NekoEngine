#ifndef __PANEL_CODE_EDITOR_H__
#define __PANEL_CODE_EDITOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ImGui\imgui.h"
#include "ImGuiColorTextEdit/TextEditor.h"

class ResourceShaderObject;

class PanelCodeEditor : public Panel
{
public:

	PanelCodeEditor(char* name);
	~PanelCodeEditor();

	bool Draw();

	void OpenShaderInCodeEditor(ResourceShaderObject* shaderObject);
	ResourceShaderObject* GetShaderObject() const;

private:

	ResourceShaderObject* shaderObject = nullptr;

	const char* fileToEdit = "";
	TextEditor editor;
};

#endif

#endif // GAME
