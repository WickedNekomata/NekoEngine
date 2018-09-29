#ifndef __PANEL_PREFERENCES_H__
#define __PANEL_PREFERENCES_H__

#include "Panel.h"

#define SCREEN_MIN_WIDTH 640
#define SCREEN_MIN_HEIGHT 480

#define STR_INPUT_SIZE 128

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

