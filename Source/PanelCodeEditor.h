#ifndef __PANEL_CODE_EDITOR_H__
#define __PANEL_CODE_EDITOR_H__

#include "Panel.h"

#ifndef GAMEMODE

#include "ImGuiColorTextEdit/TextEditor.h"

class PanelCodeEditor : public Panel
{
public:

	PanelCodeEditor(char* name);
	~PanelCodeEditor();

	bool Draw();


private:
	const char* fileToEdit = "";
	TextEditor editor;
};

#endif

#endif
