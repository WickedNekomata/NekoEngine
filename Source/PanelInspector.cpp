#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{

	return true;
}
