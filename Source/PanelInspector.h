#ifndef __PANEL_INSPECTOR_H__
#define __PANEL_INSPECTOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

class Component;

class PanelInspector : public Panel
{
public:

	PanelInspector(char* name);
	~PanelInspector();

	bool Draw();

	void DragnDropSeparatorTarget(Component* target);
};

#endif

#endif // GAME
