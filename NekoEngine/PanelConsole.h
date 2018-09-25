#ifndef __PanelConsole_H__
#define __PanelConsole_H__

#include "Panel.h"

class PanelConsole : public Panel
{
public:
	PanelConsole(char* name);
	virtual ~PanelConsole();

	virtual bool Draw();
};

#endif
