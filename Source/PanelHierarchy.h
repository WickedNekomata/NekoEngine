#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__

#include "Panel.h"

struct GameObject;

class PanelHierarchy : public Panel
{
public:

	PanelHierarchy(char* name);
	virtual ~PanelHierarchy();

	virtual bool Draw();

	void IterateAllChildren(GameObject* root);

};

#endif
