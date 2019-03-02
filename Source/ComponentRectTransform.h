#ifndef __COMPONENT_RECTTRANSFORM_H__
#define __COMPONENT_RECTTRANSFORM_H__

#define X_RECT 0
#define Y_RECT 1
#define XDIST_RECT 2
#define YDIST_RECT 3

#define LEFT_RECT 0
#define TOP_RECT 1
#define RIGHT_RECT 2
#define BOTTOM_RECT 3

#define TOPLEFT_ANCHOR 0
#define BOTTOMRIGHT_ANCHOR 1

#define X0_PERCENTAGE 0
#define X1_PERCENTAGE 1
#define Y0_PERCENTAGE 2
#define Y1_PERCENTAGE 3

#define LEFT_RECT_STR "Left"
#define TOP_RECT_STR "Top"
#define RIGHT_RECT_STR "Right"
#define BOTTOM_RECT_STR "Bottom"

#define ANCHORS_POINTS_STR "Begin\0End"

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib/include/Math/float2.h"

class ComponentRectTransform : public Component
{
public:
	ComponentRectTransform(GameObject* parent, ComponentTypes componentType = ComponentTypes::RectTransformComponent);
	ComponentRectTransform(const ComponentRectTransform& componentRectTransform, GameObject* parent, bool includeComponents = true);
	~ComponentRectTransform();

	void Update();

	void OnEditor();

	void SetRect(uint x, uint y, uint x_dist, uint y_dist);

	uint* GetRect();

	void CheckParentRect();

	void ChangeChildsRect(bool its_me = false, bool size_changed = false);

private:
	//x, y, x_dist, y_dist
	uint rectTransform[4] = { 0, 0, 100, 100 };

	//True, references top-left. False, refernces bottom-right. For every point of rect.
	bool use_margin = false;
	uint anchor[4] = {0,0,0,0};
	bool anchor_flags[4] = { false, false, false, false };
	float anchor_percenatges[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

private:
	void RecaculateAnchors();
	void RecaculateAnchors(int type);
	void RecaculatePercentage();

	uint* ui_rect = nullptr;
	uint* rectParent = nullptr;

	void ParentChanged(bool size_changed = false);
	void UseMarginChanged(bool useMargin);

	virtual uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);
	void OnUniqueEditor();
};

#endif