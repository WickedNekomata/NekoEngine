#ifndef __PANEL_GAME_H__
#define __PANEL_GAME_H__

#include "Panel.h"

class PanelGame : public Panel
{
public:

	PanelGame(char* name);
	virtual ~PanelGame();

	virtual bool Draw();
};

#endif
