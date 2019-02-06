#include "Component.h"
#include "GameObject.h"

#include "Application.h"
#include "imgui\imgui.h"

Component::Component(GameObject* parent, ComponentTypes componentType) : parent(parent), componentType(componentType)
{
	if (parent != nullptr)
		UUID = App->GenerateRandomNumber();
}

Component::~Component() {}

void Component::Update() {}

void Component::OnEditor()
{
#ifndef GAMEMODE
	char itemName[DEFAULT_BUF_SIZE];
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "Move##%u", UUID);

	ImGui::Button(itemName);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		Component* thisComponent = this;
		ImGui::SetDragDropPayload("COMPONENTS_INSPECTOR", &thisComponent, sizeof(Component));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMPONENTS_INSPECTOR"))
		{
			Component* payload_n = *(Component**)payload->Data;
			GetParent()->SwapComponents(this, payload_n);
		}
		ImGui::EndDragDropTarget();
	}

	if (componentType != ComponentTypes::MaterialComponent) 
	{
		sprintf_s(itemName, DEFAULT_BUF_SIZE, "Delete##%u", UUID);

		ImGui::SameLine();
		if (ImGui::Button(itemName)) {
			GetParent()->MarkToDeleteComponentByValue(this);
			if (componentType == ComponentTypes::MeshComponent)
				GetParent()->MarkToDeleteComponentByValue((Component*)GetParent()->materialRenderer);
		}
	}

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##isActive%u", UUID);

	bool isActive = IsActive();
	if (ImGui::Checkbox(itemName, &isActive)) { ToggleIsActive(); }

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##treeNode%u", UUID);

	if (ImGui::CollapsingHeader(itemName, ImGuiTreeNodeFlags_DefaultOpen))
		OnUniqueEditor();
#endif
}

void Component::OnUniqueEditor() {}

void Component::ToggleIsActive()
{
	isActive = !isActive;
}

bool Component::IsActive() const
{
	return isActive;
}

bool Component::IsTreeActive()
{
	bool active = isActive;

	GameObject* itGO = parent;
	while (active && itGO)
	{
		active = itGO->IsActive();
		itGO = itGO->GetParent();
	}

	return active;
}

ComponentTypes Component::GetType() const
{
	return componentType;
}

void Component::SetParent(GameObject* parent)
{
	this->parent = parent;
}

GameObject* Component::GetParent() const
{
	return parent;
}

void Component::OnSave(JSON_Object* file)
{
	json_object_set_number(file, "Type", componentType);
	OnInternalSave(file);
}

MonoObject* Component::GetMonoComponent()
{
	return monoCompHandle != 0 ? mono_gchandle_get_target(monoCompHandle) : nullptr;
}
