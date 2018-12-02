#ifndef __PANEL_CODE_EDITOR_H__
#define __PANEL_CODE_EDITOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ImGui\imgui.h"
#include "ImGuiColorTextEdit/TextEditor.h"

class PanelCodeEditor : public Panel
{
public:

	PanelCodeEditor(char* name);
	~PanelCodeEditor();

	bool Draw();

	void OpeninCodeEditor(const char* buffer);

private:

	const char* fileToEdit = "";
	TextEditor editor;
};

#endif

#endif // GAME
