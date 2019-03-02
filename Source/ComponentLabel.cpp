#include "ComponentLabel.h"

#include "ModuleUI.h"

#include "GameObject.h"
#include "Application.h"

#include "imgui/imgui.h"

ComponentLabel::ComponentLabel(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::LabelComponent)
{
	App->ui->componentsUI.push_back(this);
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel, GameObject* parent, bool includeComponents) : Component(parent, ComponentTypes::LabelComponent)
{
	if(includeComponents)
		App->ui->componentsUI.push_back(this);
}

ComponentLabel::~ComponentLabel()
{
	App->ui->componentsUI.remove(this);
}

void ComponentLabel::Update()
{
}

uint ComponentLabel::GetInternalSerializationBytes()
{
	return 0u;
}

void ComponentLabel::OnInternalSave(char *& cursor)
{
}

void ComponentLabel::OnInternalLoad(char *& cursor)
{
}

void ComponentLabel::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Text");
	ImGui::Separator();

	static char text[300] = "Edit Text";
	float sizeX = ImGui::GetWindowWidth();
	ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(sizeX, ImGui::GetTextLineHeight()*15), ImGuiInputTextFlags_AllowTabInput);

	finalText = text;
	ImGui::Text(finalText.c_str());
#endif
}
