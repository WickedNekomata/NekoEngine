#include "ComponentRectTransform.h"

#include "Application.h"
#include "ModuleWindow.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentRectTransform::ComponentRectTransform(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::RectTransformComponent)
{
}

ComponentRectTransform::ComponentRectTransform(const ComponentRectTransform & componentRectTransform) : Component(componentRectTransform.parent, ComponentTypes::RectTransformComponent)
{
	x = componentRectTransform.x;
	y = componentRectTransform.y;
	x_dist = componentRectTransform.x_dist;
	y_dist = componentRectTransform.y_dist;
}

ComponentRectTransform::~ComponentRectTransform()
{
}

void ComponentRectTransform::Update()
{
}

void ComponentRectTransform::OnEditor()
{
	uint screen_height = App->window->GetWindowHeight();
	uint screen_width = App->window->GetWindowWidth();

	uint max_xpos = screen_width - x_dist;
	uint max_ypos = screen_height - y_dist;

	uint max_xdist = screen_width - x;
	uint max_ydist = screen_height - y;

	ImGui::Text("Rect Transform");
	ImGui::PushItemWidth(50.0f);
	ImGui::Text("Positions X & Y");
	ImGui::DragScalar("##PosX", ImGuiDataType_U32, (void*)&x, 1, 0, &max_xpos, "%u", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##PosY", ImGuiDataType_U32, (void*)&y, 1, 0, &max_ypos, "%u", 1.0f);

	ImGui::Text("Size X & Y");
	ImGui::DragScalar("##SizeX", ImGuiDataType_U32, (void*)&x_dist, 1, 0, &max_xdist, "%u", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&y_dist, 1, 0, &max_ydist, "%u", 1.0f);
}
