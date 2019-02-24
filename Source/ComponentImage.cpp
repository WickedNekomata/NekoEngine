#include "ComponentImage.h"

#include "GameObject.h"
#include "ModuleUI.h"
#include "Application.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentImage::ComponentImage(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::ImageComponent)
{
	App->ui->componentsUI.push_back(this);
}

ComponentImage::ComponentImage(const ComponentImage & componentRectTransform) : Component(parent, ComponentTypes::ImageComponent)
{
	use_color_vec = componentRectTransform.use_color_vec;
	if (use_color_vec)
	{
		color[COLOR_R] = componentRectTransform.color[COLOR_R];
		color[COLOR_G] = componentRectTransform.color[COLOR_G];
		color[COLOR_B] = componentRectTransform.color[COLOR_B];
		color[COLOR_A] = componentRectTransform.color[COLOR_A];
	}
	else
		res_image = componentRectTransform.res_image;
	App->ui->componentsUI.push_back(this);

}

ComponentImage::~ComponentImage()
{
	App->ui->componentsUI.remove(this);
}

void ComponentImage::Update()
{
}

void ComponentImage::OnEditor()
{
	OnUniqueEditor();
}

const float * ComponentImage::GetColor() const
{
	return color;
}

uint ComponentImage::GetResImage()const
{
	return res_image;
}

bool ComponentImage::UseColor() const
{
	return use_color_vec;
}

uint ComponentImage::GetInternalSerializationBytes()
{
	if(use_color_vec)
		return sizeof(bool) + sizeof(float) * 4;
	else
		return sizeof(bool) + sizeof(uint);
}

void ComponentImage::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(cursor, &use_color_vec, bytes);
	cursor += bytes;

	if (use_color_vec)
	{
		bytes = sizeof(float) * 4;
		memcpy(cursor, &color, bytes);
		cursor += bytes;
	}
	else
	{
		bytes = sizeof(uint);
		memcpy(cursor, &res_image, bytes);
		cursor += bytes;
	}
}

void ComponentImage::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(bool);
	memcpy(&use_color_vec, cursor, bytes);
	cursor += bytes;
	if (use_color_vec)
	{
		bytes = sizeof(float) * 4;
		memcpy(&color, cursor, bytes);
		cursor += bytes;
	}
	else
	{
		bytes = sizeof(uint);
		memcpy(&res_image, cursor, bytes);
		cursor += bytes;
	}
}

void ComponentImage::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Image");
	ImGui::Spacing();

	ImGui::Checkbox("Use color vector", &use_color_vec);
	if (use_color_vec)
	{
		float min = 0.0f;
		float max_color = MAX_COLOR;
		float max_alpha = MAX_ALPHA;

		ImGui::Text("Color RGB with alpha");
		ImGui::DragScalar("##ColorR", ImGuiDataType_Float, (void*)&color[COLOR_R], 1.0f, &min, &max_color, "%1.f", 1.0f);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		ImGui::DragScalar("##ColorG", ImGuiDataType_Float, (void*)&color[COLOR_G], 1.0f, &min, &max_color, "%1.f", 1.0f);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		ImGui::DragScalar("##ColorB", ImGuiDataType_Float, (void*)&color[COLOR_B], 1.0f, &min, &max_color, "%1.f", 1.0f);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);		
		ImGui::DragScalar("##ColorA", ImGuiDataType_Float, (void*)&color[COLOR_A], 0.1f, &min, &max_alpha, "%0.1f", 1.0f);
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	}
	else
	{

	}
#endif
}
