#include "PanelConsole.h"
#include "Globals.h"

#include "ImGui/imgui.h"

PanelConsole::PanelConsole(char* name) : Panel(name)
{
}

PanelConsole::~PanelConsole()
{
}

bool PanelConsole::Draw()
{
	// TODO: implement a real console
	return true;
}
