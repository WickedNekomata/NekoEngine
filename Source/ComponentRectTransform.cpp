#include "ComponentRectTransform.h"

#include "Application.h"
#include "ModuleUI.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentRectTransform::ComponentRectTransform(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::RectTransformComponent)
{
	ui_rect = App->ui->GetRectUI();
	RecaculateAnchors();
}

ComponentRectTransform::ComponentRectTransform(const ComponentRectTransform & componentRectTransform) : Component(componentRectTransform.parent, ComponentTypes::RectTransformComponent)
{
	ui_rect = App->ui->GetRectUI();

	rectTransform[X_RECT] = componentRectTransform.rectTransform[X_RECT];
	rectTransform[Y_RECT] = componentRectTransform.rectTransform[Y_RECT];
	rectTransform[XDIST_RECT] = componentRectTransform.rectTransform[XDIST_RECT];
	rectTransform[YDIST_RECT] = componentRectTransform.rectTransform[YDIST_RECT];

	anchor[LEFT_RECT] = componentRectTransform.anchor[LEFT_RECT];
	anchor[TOP_RECT] = componentRectTransform.anchor[TOP_RECT];
	anchor[RIGHT_RECT] = componentRectTransform.anchor[RIGHT_RECT];
	anchor[BOTTOM_RECT] = componentRectTransform.anchor[BOTTOM_RECT];

	anchor_flags[LEFT_RECT] = componentRectTransform.anchor_flags[LEFT_RECT];
	anchor_flags[TOP_RECT] = componentRectTransform.anchor_flags[TOP_RECT];
	anchor_flags[RIGHT_RECT] = componentRectTransform.anchor_flags[RIGHT_RECT];
	anchor_flags[BOTTOM_RECT] = componentRectTransform.anchor_flags[BOTTOM_RECT];

	RecaculateAnchors();
}

ComponentRectTransform::~ComponentRectTransform()
{
}

void ComponentRectTransform::Update()
{
}

void ComponentRectTransform::OnEditor()
{
	OnUniqueEditor();
}

void ComponentRectTransform::SetRect(uint x, uint y, uint x_dist, uint y_dist)
{
	rectTransform[X_RECT] = x;
	rectTransform[Y_RECT] = y;
	rectTransform[XDIST_RECT] = x_dist;
	rectTransform[YDIST_RECT] = y_dist;
}

const uint * ComponentRectTransform::GetRect() const
{
	return rectTransform;
}

void ComponentRectTransform::RecaculateAnchors()
{
	if (anchor_flags[LEFT_RECT] == TOPLEFT_ANCHOR)
		anchor[LEFT_RECT] = rectTransform[X_RECT] - ui_rect[UI_XRECT];
	else
		anchor[LEFT_RECT] = ui_rect[UI_WIDTHRECT] - rectTransform[X_RECT];

	if (anchor_flags[TOP_RECT] == TOPLEFT_ANCHOR)
		anchor[TOP_RECT] = rectTransform[Y_RECT] - ui_rect[UI_YRECT];
	else
		anchor[TOP_RECT] = ui_rect[UI_HEIGHTRECT] - rectTransform[Y_RECT];

	if (anchor_flags[RIGHT_RECT] == TOPLEFT_ANCHOR)
		anchor[RIGHT_RECT] = (rectTransform[X_RECT] + rectTransform[XDIST_RECT]) - ui_rect[UI_XRECT];
	else
		anchor[RIGHT_RECT] = ui_rect[UI_WIDTHRECT]  - (rectTransform[X_RECT] + rectTransform[XDIST_RECT]);

	if (anchor_flags[BOTTOM_RECT] == TOPLEFT_ANCHOR)
		anchor[BOTTOM_RECT] = (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]) - ui_rect[UI_XRECT];
	else
		anchor[BOTTOM_RECT] = ui_rect[UI_HEIGHTRECT] - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]);
}

uint ComponentRectTransform::GetInternalSerializationBytes()
{
	return sizeof(uint) * 8 + sizeof(bool) * 4;
}

void ComponentRectTransform::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(uint) * 4;
	memcpy(cursor, &rectTransform, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(cursor, &anchor, bytes);
	cursor += bytes;

	bytes = sizeof(bool) * 4;
	memcpy(cursor, &anchor_flags, bytes);
	cursor += bytes;
}

void ComponentRectTransform::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(uint) * 4;
	memcpy(&rectTransform, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * 4;
	memcpy(&anchor, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool) * 4;
	memcpy(&anchor_flags, cursor, bytes);
	cursor += bytes;
}

void ComponentRectTransform::OnUniqueEditor()
{
#ifndef GAMEMODE

	ImGui::Text("Rect Transform");
	ImGui::Spacing();

	uint screen_height = ui_rect[UI_HEIGHTRECT];
	uint screen_width = ui_rect[UI_WIDTHRECT];

	uint max_xpos = screen_width - rectTransform[XDIST_RECT];
	uint max_ypos = screen_height - rectTransform[YDIST_RECT];

	uint max_xdist = screen_width - rectTransform[X_RECT];
	uint max_ydist = screen_height - rectTransform[Y_RECT];

	bool needed_recalculate = false;

	ImGui::PushItemWidth(50.0f);

	ImGui::Text("Positions X & Y");
	if (ImGui::DragScalar("##PosX", ImGuiDataType_U32, (void*)&rectTransform[X_RECT], 1, 0, &max_xpos, "%u", 1.0f))
		needed_recalculate = true;
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##PosY", ImGuiDataType_U32, (void*)&rectTransform[Y_RECT], 1, 0, &max_ypos, "%u", 1.0f))
		needed_recalculate = true;

	ImGui::Text("Size X & Y");
	if (ImGui::DragScalar("##SizeX", ImGuiDataType_U32, (void*)&rectTransform[XDIST_RECT], 1, 0, &max_xdist, "%u", 1.0f))
		needed_recalculate = true;
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&rectTransform[YDIST_RECT], 1, 0, &max_ydist, "%u", 1.0f))
		needed_recalculate = true;

	ImGui::PushItemWidth(150.0f);
	ImGui::Text("Anchor");
	ImGui::PushItemWidth(50.0f);

	int current_anchor_flag = (int)anchor_flags[LEFT_RECT];
	if (ImGui::Combo(LEFT_RECT_STR, &current_anchor_flag, ANCHORS_POINTS_STR))
	{
		anchor_flags[LEFT_RECT] = current_anchor_flag;
		needed_recalculate = true;
	}

	current_anchor_flag = (int)anchor_flags[TOP_RECT];
	if (ImGui::Combo(TOP_RECT_STR, (int*)&current_anchor_flag, ANCHORS_POINTS_STR))
	{
		anchor_flags[TOP_RECT] = current_anchor_flag;
		needed_recalculate = true;
	}

	current_anchor_flag = (int)anchor_flags[RIGHT_RECT];
	if (ImGui::Combo(RIGHT_RECT_STR, (int*)&current_anchor_flag, ANCHORS_POINTS_STR))
	{
		anchor_flags[RIGHT_RECT] = current_anchor_flag;
		needed_recalculate = true;
	}

	current_anchor_flag = (int)anchor_flags[BOTTOM_RECT];
	if (ImGui::Combo(BOTTOM_RECT_STR, (int*)&current_anchor_flag, ANCHORS_POINTS_STR))
	{
		anchor_flags[BOTTOM_RECT] = current_anchor_flag;
		needed_recalculate = true;
	}

	if (needed_recalculate)
		RecaculateAnchors();

	ImGui::PushItemWidth(150.0f);
	ImGui::Text("Margin");
	ImGui::PushItemWidth(50.0f);

	ImGui::Text("Left %u", anchor[LEFT_RECT]);
	ImGui::Text("Top %u", anchor[TOP_RECT]);
	ImGui::Text("Right %u", anchor[RIGHT_RECT]);
	ImGui::Text("Bottom %u", anchor[BOTTOM_RECT]);
#endif
}
