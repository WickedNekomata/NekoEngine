#ifndef __PanelPreferences_H__
#define __PanelPreferences_H__

#include "Panel.h"

class PanelPreferences : public Panel
{
public:
	PanelPreferences(char* name);
	virtual ~PanelPreferences();

	virtual bool Draw();

	void HardwareNode();
	void ApplicationNode();
};

#endif

