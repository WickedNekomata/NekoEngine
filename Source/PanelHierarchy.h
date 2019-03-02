#ifndef __PANEL_HIERARCHY_H__
#define __PANEL_HIERARCHY_H__



#ifndef GAMEMODE

#include "Panel.h"

class GameObject;

class PanelHierarchy : public Panel
{
public:

	PanelHierarchy(const char* name);
	~PanelHierarchy();

	bool Draw();

private:

	void IterateAllChildren(GameObject* root) const;
	void AtGameObjectPopUp(GameObject* child) const;
	void SetGameObjectDragAndDrop(GameObject* SourceTarget) const;
	void SetGameObjectDragAndDropSource(GameObject* source) const;
	void SetGameObjectDragAndDropTarget(GameObject* target) const;
};

#endif

#endif // GAME
