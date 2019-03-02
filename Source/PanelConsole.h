#ifndef __PANEL_CONSOLE_H__
#define __PANEL_CONSOLE_H__



#ifndef GAMEMODE

#include "Panel.h"

#include "ImGui\imgui.h"

class PanelConsole : public Panel
{
public:

	PanelConsole(const char* name);
	~PanelConsole();

	bool Draw();

	void AddLog(const char* log);

	void Clear();

private:

	ImGuiTextBuffer buf;
	ImGuiTextFilter filter;
	ImVector<int> lineOffsets;
	bool scrollToBottom = true;
};

#endif

#endif // GAME
