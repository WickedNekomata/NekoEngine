#ifndef __PANEL_PREFERENCES_H__
#define __PANEL_PREFERENCES_H__

#include "Panel.h"

class PanelPreferences : public Panel
{
public:

	PanelPreferences(char* name);
	virtual ~PanelPreferences();

	virtual bool Draw();

private:

	void HardwareNode() const;
	void ApplicationNode() const;
	void WindowNode() const;
};

#endif

