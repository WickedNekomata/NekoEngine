#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__

#include "GameMode.h"

#ifndef GAMEMODE

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
	void SetGameObjectDragAndDropSource(GameObject* source);
	void SetGameObjectDragAndDropTarget(GameObject* target);
};

#endif

#endif // GAME
