#ifndef __PANEL_IMPORT_H__
#define __PANEL_IMPORT_H__

#include "Panel.h"

class PanelImport : public Panel
{
public:

	PanelImport(char* name);
	virtual ~PanelImport();

	virtual bool Draw();
};

#endif
