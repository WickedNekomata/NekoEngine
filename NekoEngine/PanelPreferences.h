#ifndef __PANEL_PREFERENCES_H__
#define __PANEL_PREFERENCES_H__

#include "Panel.h"

class PanelPreferences : public Panel
{
public:
	PanelPreferences(char* name);
	virtual ~PanelPreferences();

	virtual bool Draw();

	void HardwareNode();
	void ApplicationNode();
	void WindowNode();
};

#endif

