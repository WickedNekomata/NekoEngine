#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__

#include "Panel.h"

struct GameObject;

class PanelHierarchy : public Panel
{
public:

	PanelHierarchy(char* name);
	~PanelHierarchy();

	bool Draw();

private:

	void IterateAllChildren(GameObject* root);
	void AtGameObjectPopUp(GameObject* child);
	void SetGameObjectDragAndDrop(GameObject* SourceTarget);
};

#endif
