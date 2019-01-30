#ifndef __PANEL_GAME_TIME_H__
#define __PANEL_GAME_TIME_H__

#include "Panel.h"

#ifndef GAMEMODE

class PanelSimulatedTime : public Panel
{
public:

	PanelSimulatedTime(char* name);
	~PanelSimulatedTime();

	bool Draw();
};

#endif

#endif
