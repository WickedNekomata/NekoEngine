#include "ComponentImage.h"

#include "Application.h"
#include "ModuleUI.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"

#include "GameObject.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

ComponentImage::ComponentImage(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::ImageComponent)
{
	App->ui->componentsUI.push_back(this);
}

ComponentImage::ComponentImage(const ComponentImage & componentRectTransform, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::ImageComponent)
{
	use_color_vec = componentRectTransform.use_color_vec;
	if (use_color_vec)
		memcpy(color, componentRectTransform.color, sizeof(float) * 4);
	else
	{
		res_image = componentRectTransform.res_image;
		if(includeComponents)
			App->res->SetAsUsed(res_image);
	}
	if (includeComponents)
		App->ui->componentsUI.push_back(this);
}

ComponentImage::~ComponentImage()
{
	App->ui->componentsUI.remove(this);
}

void ComponentImage::Update()
{
}

const float * ComponentImage::GetColor() const
{
	return color;
}

uint ComponentImage::GetResImage()const
{
	ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(res_image);
	if (texture)
		return texture->GetId();
	else
		return 0;
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

		App->res->SetAsUsed(res_image);
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

		float color_r = color[COLOR_R] * 255.f;
		float color_g = color[COLOR_G] * 255.f;
		float color_b = color[COLOR_B] * 255.f;

		ImGui::Text("Color RGB with alpha");
		if (ImGui::DragScalar("##ColorR", ImGuiDataType_Float, (void*)&color_r, 1.0f, &min, &max_color, "%1.f", 1.0f))
			color[COLOR_R] = color_r / 255.f;
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if(ImGui::DragScalar("##ColorG", ImGuiDataType_Float, (void*)&color_g, 1.0f, &min, &max_color, "%1.f", 1.0f))
			color[COLOR_G] = color_g / 255.f;
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
		if(ImGui::DragScalar("##ColorB", ImGuiDataType_Float, (void*)&color_b, 1.0f, &min, &max_color, "%1.f", 1.0f))
			color[COLOR_B] = color_b / 255.f;
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);		
		if (ImGui::DragScalar("##ColorA", ImGuiDataType_Float, (void*)&color[COLOR_A], 0.1f, &min, &max_alpha, "%0.1f", 1.0f))
		ImGui::SameLine(); ImGui::PushItemWidth(50.0f);
	}
	else
	{
		ResourceTexture* texture = nullptr;
		if (res_image != 0)
			texture = (ResourceTexture*)App->res->GetResource(res_image);
		ImGui::Button(texture == nullptr ? "Empty texture" : texture->GetName(), ImVec2(150.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res_image);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				App->res->SetAsUnused(res_image);
				res_image = *(uint*)payload->Data;
				App->res->SetAsUsed(res_image);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}
