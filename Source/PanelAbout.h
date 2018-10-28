#ifndef __PANEL_ABOUT_H__
#define __PANEL_ABOUT_H__

#include "Panel.h"

class PanelAbout : public Panel
{
public:

	PanelAbout(char* name);
	~PanelAbout();

	bool Draw();
};

#endif
