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

	ImGui::Text("Rect");
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##PosX", ImGuiDataType_U32, (void*)&x, 1, 0, &screen_width - x_dist, "%u", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##PosY", ImGuiDataType_U32, (void*)&y, 1, 0, &screen_height - y_dist, "%u", 1.0f); 

	ImGui::DragScalar("##SizeX", ImGuiDataType_U32, (void*)&x_dist, 1, 0, &screen_width - x, "%u", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(50.0f);
	ImGui::DragScalar("##SizeY", ImGuiDataType_U32, (void*)&y_dist, 1, 0, &screen_height - y, "%u", 1.0f);
}
