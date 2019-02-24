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
		//	GetParent()->SwapComponents(this, payload_n);
		}
		ImGui::EndDragDropTarget();
	}

	if (componentType != ComponentTypes::MaterialComponent) 
	{
		sprintf_s(itemName, DEFAULT_BUF_SIZE, "Delete##%u", UUID);

		ImGui::SameLine();
		if (ImGui::Button(itemName)) {
			GetParent()->DestroyComponent(this);
		}
	}

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "Is active##isActive%u", UUID);

	bool isActive = IsActive();
	if (ImGui::Checkbox(itemName, &isActive)) { ToggleIsActive(); }

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##treeNode%u", UUID);

	OnUniqueEditor();
#endif
}

void Component::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Stop:
		case System_Event_Type::ScriptingDomainReloaded:
		{
			this->monoCompHandle = 0;
			break;
		}
	}
}

uint Component::GetSerializationBytes()
{
	// uuid + type + active + internal
	return sizeof(int) + sizeof(uint) + sizeof(bool) + GetInternalSerializationBytes();
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

void Component::OnSave(char*& cursor)
{
	size_t bytes = sizeof(int);

	memcpy(cursor, &componentType, bytes);
	cursor += bytes;

	bytes = sizeof(uint);

	memcpy(cursor, &UUID, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &isActive, bytes);
	cursor += bytes;

	OnInternalSave(cursor);
}

void Component::OnLoad(char*& cursor)
{
	size_t bytes = sizeof(uint);

	memcpy(&UUID, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&isActive, cursor, bytes);
	cursor += bytes;

	OnInternalLoad(cursor);
}

MonoObject* Component::GetMonoComponent()
{
	return monoCompHandle != 0 ? mono_gchandle_get_target(monoCompHandle) : nullptr;
}
