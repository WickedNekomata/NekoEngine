#include "ComponentRectTransform.h"

#include "Application.h"
#include "ModuleUI.h"

#include "GameObject.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentRectTransform::ComponentRectTransform(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::RectTransformComponent)
{
	App->ui->componentsUI.push_back(this);
	ui_rect = App->ui->GetRectUI();

	Component* rect = nullptr;
	if (parent->GetParent() != nullptr && (rect = parent->GetParent()->GetComponent(ComponentTypes::RectTransformComponent)) != nullptr)
	{
		rectParent = ((ComponentRectTransform*)rect)->GetRect();

		ParentChanged();
	}

	RecaculateAnchors();
	RecaculatePercentage();
}

ComponentRectTransform::ComponentRectTransform(const ComponentRectTransform & componentRectTransform) : Component(componentRectTransform.parent, ComponentTypes::RectTransformComponent)
{
	App->ui->componentsUI.push_back(this);
	ui_rect = App->ui->GetRectUI();

	memcpy(rectTransform, componentRectTransform.rectTransform, sizeof(uint) * 4);
	memcpy(anchor, componentRectTransform.anchor, sizeof(uint) * 4);
	memcpy(anchor_flags, componentRectTransform.anchor_flags, sizeof(bool) * 4);

	Component* rect = nullptr;
	if (parent->GetParent() != nullptr && (rect = parent->GetParent()->GetComponent(ComponentTypes::RectTransformComponent)) != nullptr)
	{
		rectParent = ((ComponentRectTransform*)rect)->GetRect();

		ParentChanged();
	}

	RecaculateAnchors();
	RecaculatePercentage();
}

ComponentRectTransform::~ComponentRectTransform()
{
	App->ui->componentsUI.remove(this);
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

	RecaculateAnchors();
	RecaculatePercentage();
}

uint* ComponentRectTransform::GetRect()
{
	return rectTransform;
}

void ComponentRectTransform::CheckParentRect()
{
	Component* rect = nullptr;
	if (parent->GetParent() != nullptr && (rect = parent->GetParent()->GetComponent(ComponentTypes::RectTransformComponent)) != nullptr)
	{
		rectParent = ((ComponentRectTransform*)rect)->GetRect();

		ParentChanged();
	}

	RecaculateAnchors();
	RecaculatePercentage();
}

void ComponentRectTransform::ChangeChildsRect(bool its_me, bool size_changed)
{
	if (!its_me)
	{
		if(use_margin)
			ParentChanged(false);
		else
			ParentChanged(size_changed);
	}

	std::vector<GameObject*> childs;
	parent->GetChildrenVector(childs);
	std::reverse(childs.begin(), childs.end());
	for (GameObject* c_go : childs)
	{
		if(c_go != parent)
			((ComponentRectTransform*)c_go->GetComponent(ComponentTypes::RectTransformComponent))->ChangeChildsRect(false, size_changed);
	}
}

void ComponentRectTransform::ParentChanged(bool size_changed)
{
	if (size_changed)
	{
		rectTransform[X_RECT] = (uint)(anchor_percenatges[X0_PERCENTAGE] * (float)rectParent[XDIST_RECT]) + rectParent[X_RECT];
		rectTransform[XDIST_RECT] = rectParent[XDIST_RECT] - ((rectTransform[X_RECT] - rectParent[X_RECT]) + (uint)(anchor_percenatges[X1_PERCENTAGE] * (float)rectParent[XDIST_RECT]));
		rectTransform[Y_RECT] = (uint)(anchor_percenatges[Y0_PERCENTAGE] * (float)rectParent[YDIST_RECT]) + rectParent[Y_RECT];
		rectTransform[YDIST_RECT] = rectParent[YDIST_RECT] - ((rectTransform[Y_RECT] - rectParent[Y_RECT]) + (uint)(anchor_percenatges[Y1_PERCENTAGE] * (float)rectParent[YDIST_RECT]));
	
		RecaculateAnchors();
	}
	else
	{
		if (anchor_flags[LEFT_RECT] == TOPLEFT_ANCHOR)
			rectTransform[X_RECT] = rectParent[X_RECT] + anchor[LEFT_RECT];
		else
			rectTransform[X_RECT] = rectParent[X_RECT] + rectParent[XDIST_RECT] - anchor[LEFT_RECT];

		if (anchor_flags[TOP_RECT] == TOPLEFT_ANCHOR)
			rectTransform[Y_RECT] = rectParent[Y_RECT] + anchor[TOP_RECT];
		else
			rectTransform[Y_RECT] = rectParent[Y_RECT] + rectParent[YDIST_RECT] - anchor[TOP_RECT];

		if (anchor_flags[RIGHT_RECT] == TOPLEFT_ANCHOR)
			rectTransform[XDIST_RECT] = rectParent[X_RECT] + anchor[RIGHT_RECT] - rectTransform[X_RECT];
		else
			rectTransform[XDIST_RECT] = rectParent[X_RECT] + rectParent[XDIST_RECT] - anchor[RIGHT_RECT] - rectTransform[X_RECT];

		if (anchor_flags[BOTTOM_RECT] == TOPLEFT_ANCHOR)
			rectTransform[YDIST_RECT] = rectParent[Y_RECT] + anchor[BOTTOM_RECT] - rectTransform[Y_RECT];
		else
			rectTransform[YDIST_RECT] = rectParent[Y_RECT] + rectParent[YDIST_RECT] - anchor[BOTTOM_RECT] - rectTransform[Y_RECT];

		RecaculatePercentage();
	}
}

void ComponentRectTransform::UseMarginChanged(bool useMargin)
{
	use_margin = useMargin;

	std::vector<GameObject*> childs;
	parent->GetChildrenVector(childs);
	std::reverse(childs.begin(), childs.end());
	for (GameObject* c_go : childs)
	{
		if (c_go != parent)
			((ComponentRectTransform*)c_go->GetComponent(ComponentTypes::RectTransformComponent))->UseMarginChanged(useMargin);
	}
}

void ComponentRectTransform::RecaculateAnchors()
{
	if (rectParent != nullptr)
	{
		if (anchor_flags[LEFT_RECT] == TOPLEFT_ANCHOR)
			anchor[LEFT_RECT] = rectTransform[X_RECT] - rectParent[X_RECT];
		else
			anchor[LEFT_RECT] = (rectParent[X_RECT] + rectParent[XDIST_RECT]) - rectTransform[X_RECT];

		if (anchor_flags[TOP_RECT] == TOPLEFT_ANCHOR)
			anchor[TOP_RECT] = rectTransform[Y_RECT] - rectParent[UI_YRECT];
		else
			anchor[TOP_RECT] = (rectParent[Y_RECT] + rectParent[YDIST_RECT]) - rectTransform[Y_RECT];

		if (anchor_flags[RIGHT_RECT] == TOPLEFT_ANCHOR)
			anchor[RIGHT_RECT] = (rectTransform[X_RECT] + rectTransform[XDIST_RECT]) - rectParent[X_RECT];
		else
			anchor[RIGHT_RECT] = (rectParent[X_RECT] + rectParent[XDIST_RECT]) - (rectTransform[X_RECT] + rectTransform[XDIST_RECT]);

		if (anchor_flags[BOTTOM_RECT] == TOPLEFT_ANCHOR)
			anchor[BOTTOM_RECT] = (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]) - rectParent[Y_RECT];
		else
			anchor[BOTTOM_RECT] = (rectParent[Y_RECT] + rectParent[YDIST_RECT]) - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]);
	}
	else
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
			anchor[RIGHT_RECT] = ui_rect[UI_WIDTHRECT] - (rectTransform[X_RECT] + rectTransform[XDIST_RECT]);

		if (anchor_flags[BOTTOM_RECT] == TOPLEFT_ANCHOR)
			anchor[BOTTOM_RECT] = (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]) - ui_rect[UI_XRECT];
		else
			anchor[BOTTOM_RECT] = ui_rect[UI_HEIGHTRECT] - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]);
	}
}

void ComponentRectTransform::RecaculateAnchors(int type)
{
	if (TOPLEFT_ANCHOR == type)
	{
		if (rectParent != nullptr)
		{
			rectTransform[X_RECT] = anchor[LEFT_RECT] + rectParent[X_RECT];
			rectTransform[Y_RECT] = anchor[TOP_RECT] + rectParent[Y_RECT];
			anchor[RIGHT_RECT] = rectParent[XDIST_RECT] - (rectTransform[X_RECT] + rectTransform[XDIST_RECT]);
			anchor[BOTTOM_RECT] = rectParent[YDIST_RECT] - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]);
		}
		else
		{
			rectTransform[X_RECT] = anchor[LEFT_RECT] + ui_rect[UI_XRECT];
			rectTransform[Y_RECT] = anchor[TOP_RECT] + ui_rect[UI_YRECT];
			anchor[RIGHT_RECT] =  ui_rect[UI_WIDTHRECT] - (rectTransform[X_RECT] + rectTransform[XDIST_RECT]);
			anchor[BOTTOM_RECT] = ui_rect[UI_HEIGHTRECT] - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]);
		}
	}
	else
	{
		if (rectParent != nullptr)
		{
			rectTransform[X_RECT] = (rectParent[XDIST_RECT] + rectParent[X_RECT]) - anchor[RIGHT_RECT] - rectTransform[XDIST_RECT];
			rectTransform[Y_RECT] = (rectParent[YDIST_RECT]  + rectParent[YDIST_RECT]) - anchor[BOTTOM_RECT] - rectTransform[YDIST_RECT];
			anchor[LEFT_RECT] = (rectTransform[X_RECT] + rectTransform[XDIST_RECT]) + anchor[RIGHT_RECT];
			anchor[TOP_RECT] = (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]) + anchor[BOTTOM_RECT];
		}
		else
		{
			rectTransform[X_RECT] = (ui_rect[UI_WIDTHRECT] + ui_rect[UI_XRECT]) - anchor[RIGHT_RECT] - rectTransform[XDIST_RECT];
			rectTransform[Y_RECT] = (ui_rect[UI_HEIGHTRECT] + ui_rect[UI_YRECT]) - anchor[BOTTOM_RECT] - rectTransform[YDIST_RECT];
			anchor[LEFT_RECT] = (rectTransform[X_RECT] + rectTransform[XDIST_RECT]) + anchor[RIGHT_RECT];
			anchor[TOP_RECT] = (rectTransform[Y_RECT] + rectTransform[YDIST_RECT]) + anchor[BOTTOM_RECT];
		}
	}

	ChangeChildsRect(true);
}

void ComponentRectTransform::RecaculatePercentage()
{
	if (rectParent != nullptr)
	{
		anchor_percenatges[X0_PERCENTAGE] = (float)(rectTransform[X_RECT] - rectParent[X_RECT]) / (float)rectParent[XDIST_RECT];
		anchor_percenatges[X1_PERCENTAGE] = (float)((rectParent[X_RECT] + rectParent[XDIST_RECT]) - (rectTransform[X_RECT] + rectTransform[XDIST_RECT])) / (float)rectParent[XDIST_RECT];
		anchor_percenatges[Y0_PERCENTAGE] = (float)(rectTransform[Y_RECT] - rectParent[Y_RECT]) / (float)rectParent[YDIST_RECT];
		anchor_percenatges[Y1_PERCENTAGE] = (float)((rectParent[Y_RECT] + rectParent[YDIST_RECT]) - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT])) / (float)rectParent[YDIST_RECT];
	}
	else
	{
		anchor_percenatges[X0_PERCENTAGE] = (float)(rectTransform[X_RECT] - ui_rect[UI_XRECT]) / (float)ui_rect[UI_WIDTHRECT];
		anchor_percenatges[X1_PERCENTAGE] = (float)((ui_rect[UI_XRECT] + ui_rect[UI_WIDTHRECT]) - (rectTransform[X_RECT] + rectTransform[XDIST_RECT])) / (float)ui_rect[UI_WIDTHRECT];
		anchor_percenatges[Y0_PERCENTAGE] = (float)(rectTransform[Y_RECT] - ui_rect[UI_YRECT]) / (float)ui_rect[UI_HEIGHTRECT];
		anchor_percenatges[Y1_PERCENTAGE] = (float)((ui_rect[UI_YRECT] + ui_rect[UI_HEIGHTRECT]) - (rectTransform[Y_RECT] + rectTransform[YDIST_RECT])) / (float)ui_rect[UI_HEIGHTRECT];
	}
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

	uint screen_height = 0;
	uint screen_width = 0;

	uint min_xpos = 0;
	uint min_ypos = 0;

	uint max_xpos = 0;
	uint max_ypos = 0;

	uint max_xdist = 0;
	uint max_ydist = 0;

	if (rectParent != nullptr)
	{
		min_xpos = rectParent[X_RECT];
		min_ypos = rectParent[Y_RECT];

		screen_width = rectParent[X_RECT] + rectParent[XDIST_RECT];
		screen_height = rectParent[Y_RECT] + rectParent[YDIST_RECT];

		max_xpos = screen_width - rectTransform[XDIST_RECT];
		max_ypos = screen_height - rectTransform[YDIST_RECT];

		max_xdist = screen_width - rectTransform[X_RECT];
		max_ydist = screen_height - rectTransform[Y_RECT];
	}
	else
	{
		screen_height = ui_rect[UI_HEIGHTRECT];
		screen_width = ui_rect[UI_WIDTHRECT];

		max_xpos = screen_width - rectTransform[XDIST_RECT];
		max_ypos = screen_height - rectTransform[YDIST_RECT];

		max_xdist = screen_width - rectTransform[X_RECT];
		max_ydist = screen_height - rectTransform[Y_RECT];
	}
	bool needed_recalculate = false;
	bool size_changed = false;

	ImGui::PushItemWidth(50.0f);

	ImGui::Text("Positions X & Y");
	if (ImGui::DragScalar("##PosX", ImGuiDataType_U32, (void*)&rectTransform[X_RECT], 1, &min_xpos, &max_xpos, "%u", 1.0f))
		needed_recalculate = true;
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##PosY", ImGuiDataType_U32, (void*)&rectTransform[Y_RECT], 1, &min_ypos, &max_ypos, "%u", 1.0f))
		needed_recalculate = true;

	ImGui::Text("Size X & Y");
	if (ImGui::DragScalar("##SizeX", ImGuiDataType_U32, (void*)&rectTransform[XDIST_RECT], 1, 0, &max_xdist, "%u", 1.0f))
	{
		needed_recalculate = true;
		size_changed = true;
	}
	ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	if (ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&rectTransform[YDIST_RECT], 1, 0, &max_ydist, "%u", 1.0f))
	{
		needed_recalculate = true;
		size_changed = true;
	}

	if (needed_recalculate)
	{
		RecaculatePercentage();
		ChangeChildsRect(true, size_changed);
	}

	ImGui::Checkbox("Use margin", &use_margin);
	ImGui::SameLine();
	if(ImGui::Button("Set to childs"))
		UseMarginChanged(use_margin);

	if (use_margin)
	{
		ImGui::PushItemWidth(150.0f);
		ImGui::Text("Anchor");
		ImGui::PushItemWidth(50.0f);

		int current_anchor_flag = (int)anchor_flags[LEFT_RECT];
		ImGui::Text("Reference points");
		ImGui::Text("Begin - Top/Left");
		ImGui::Text("End - Bottom/Right");
		if (ImGui::Combo("Using: ", &current_anchor_flag, ANCHORS_POINTS_STR))
		{
			anchor_flags[LEFT_RECT] = current_anchor_flag;
			anchor_flags[TOP_RECT] = current_anchor_flag;
			anchor_flags[RIGHT_RECT] = current_anchor_flag;
			anchor_flags[BOTTOM_RECT] = current_anchor_flag;

			RecaculateAnchors();
		}
		/*
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
		*/


		needed_recalculate = false;

		ImGui::PushItemWidth(50.0f);

		ImGui::Text("Margin");
		if (current_anchor_flag == TOPLEFT_ANCHOR)
		{
			uint max_leftAnhor = 0;
			uint max_topAnchor = 0;
			if (rectParent)
			{
				max_leftAnhor = (rectParent[X_RECT] + rectParent[XDIST_RECT]) - rectTransform[XDIST_RECT];
				max_topAnchor = (rectParent[Y_RECT] + rectParent[YDIST_RECT]) - rectTransform[YDIST_RECT];
			}
			else
			{
				max_leftAnhor = (ui_rect[UI_XRECT] + ui_rect[UI_WIDTHRECT]) - rectTransform[XDIST_RECT];
				max_topAnchor = (ui_rect[UI_YRECT] + ui_rect[UI_HEIGHTRECT]) - rectTransform[YDIST_RECT];
			}

			ImGui::Text("Left/Top");
			if (ImGui::DragScalar("##MLeft", ImGuiDataType_U32, (void*)&anchor[LEFT_RECT], 1, 0, &max_leftAnhor, "%u", 1.0f))
				needed_recalculate = true;
			ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragScalar("##MTop", ImGuiDataType_U32, (void*)&anchor[TOP_RECT], 1, 0, &max_topAnchor, "%u", 1.0f))
				needed_recalculate = true;

			if (needed_recalculate)
				RecaculateAnchors(TOPLEFT_ANCHOR);
		}
		else
		{
			uint max_rightAnhor = 0;
			uint max_bottomAnchor = 0;

			if (rectParent)
			{
				max_rightAnhor = rectParent[XDIST_RECT] - (rectParent[X_RECT] + rectTransform[XDIST_RECT]);
				max_bottomAnchor = rectParent[YDIST_RECT] - (rectParent[Y_RECT] + rectTransform[YDIST_RECT]);
			}
			else
			{
				max_rightAnhor = ui_rect[UI_WIDTHRECT] - (ui_rect[UI_XRECT] + rectTransform[XDIST_RECT]);
				max_bottomAnchor = ui_rect[UI_HEIGHTRECT] - (ui_rect[UI_YRECT] + rectTransform[YDIST_RECT]);
			}

			ImGui::Text("Right/Bottom");
			if (ImGui::DragScalar("##MRight", ImGuiDataType_U32, (void*)&anchor[RIGHT_RECT], 1, 0, &max_rightAnhor, "%u", 1.0f))
				needed_recalculate = true;
			ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
			if (ImGui::DragScalar("##MBottom", ImGuiDataType_U32, (void*)&anchor[BOTTOM_RECT], 1, 0, &max_bottomAnchor, "%u", 1.0f))
				needed_recalculate = true;

			if (needed_recalculate)
				RecaculateAnchors(BOTTOMRIGHT_ANCHOR);
		}
	}

#endif
}
