#ifndef __PANEL_EDIT_H__
#define __PANEL_EDIT_H__

#include "Panel.h"

#ifndef GAMEMODE

class PanelEdit : public Panel
{
public:

	PanelEdit(char* name);
	~PanelEdit();

	bool Draw();
};

#endif

#endif
