#include "PanelHierarchy.h"

#ifndef GAMEMODE

#include "Application.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "SDL\include\SDL_scancode.h"
#include "ModuleGOs.h"
#include "ImGui\imgui.h"
#include "imgui\imgui_internal.h"

#include "ComponentTransform.h"

PanelHierarchy::PanelHierarchy(char* name) : Panel(name) {}

PanelHierarchy::~PanelHierarchy() {}

bool PanelHierarchy::Draw()
{
	ImGuiWindowFlags hierarchyFlags = 0;
	hierarchyFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, hierarchyFlags))
	{
		static char sceneName[INPUT_BUF_SIZE];
		strcpy_s(sceneName, IM_ARRAYSIZE(sceneName), App->GOs->nameScene);
		ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushItemWidth(100.0f);
		if (ImGui::InputText("##sceneName", sceneName, IM_ARRAYSIZE(sceneName), inputFlag))
			strcpy_s((char*)App->GOs->nameScene, DEFAULT_BUF_SIZE, sceneName);

		ImGui::Separator();
		ImGui::Dummy(ImVec2(ImGui::GetWindowSize().x, 2.0f));
		GameObject* root = App->scene->root;

		// DRAGNDROP FOR ROOT AT SEPARATOR
		SetGameObjectDragAndDropTarget(App->scene->root);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Create Empty"))
			{
				App->GOs->CreateGameObject("GameObject", root);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		IterateAllChildren(root);		
	}
	ImGui::End();
	ImRect rect(ImGui::GetWindowPos(), ImGui::GetWindowSize());
	if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetID(name)))
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DROP_PREFAB_TO_GAME"))
		{
			char* payload_n = (char*)payload->Data;
			App->GOs->LoadScene(payload_n);
		}
		ImGui::EndDragDropTarget();
	}

	return true;
}

void PanelHierarchy::IterateAllChildren(GameObject* root)
{
	ImGuiTreeNodeFlags treeNodeFlags;

	if (root->HasChildren())
	{
		for (int i = 0; i < root->GetChildrenLength(); ++i)
		{
			// TODO ADD DRAG AND DROP DUMMY
			//ImGui::Dummy(); // DRAG AND DROP
			// ReorderGameObject method at gos

			GameObject* child = root->GetChild(i);

			char name[DEFAULT_BUF_SIZE];
			sprintf_s(name, DEFAULT_BUF_SIZE, "%s##%u", child->GetName(), child->GetUUID());

			if (child->HasChildren())
			{
				treeNodeFlags = 0;
				treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

				if (App->scene->selectedObject == child)
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

				bool treeNodeOpened = false;
				if (ImGui::TreeNodeEx(name, treeNodeFlags))
					treeNodeOpened = true;

				SetGameObjectDragAndDrop(child);
				AtGameObjectPopUp(child);

				if (App->scene->selectedObject == child && App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
				{
					App->scene->selectedObject = CurrentSelection::SelectedType::null;
					App->GOs->DeleteGameObject(child);
				}

				if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
					DESTROYANDSET(child);

				if (treeNodeOpened) 
				{
					IterateAllChildren(child);
					ImGui::TreePop();
				}
			}
			else
			{
				treeNodeFlags = 0;
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

				if (App->scene->selectedObject == child)
					treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

				ImGui::TreeNodeEx(name, treeNodeFlags);
				ImGui::TreePop();

				SetGameObjectDragAndDrop(child);
				
				AtGameObjectPopUp(child);

				if (App->scene->selectedObject == child && App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
				{
					App->scene->selectedObject = CurrentSelection::SelectedType::null;
					App->GOs->DeleteGameObject(child);
				}
			
				if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
					DESTROYANDSET(child);				
			}
		}
	}
}

void PanelHierarchy::AtGameObjectPopUp(GameObject* child)
{
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Create Empty")) 
		{
			App->GOs->CreateGameObject("GameObject", child);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("Delete")) 
		{
			if (child->EqualsToChildrenOrMe(App->scene->selectedObject.Get()))
				App->scene->selectedObject = CurrentSelection::SelectedType::null;
			App->GOs->DeleteGameObject(child);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::SetGameObjectDragAndDrop(GameObject* sourceTarget) const
{
	SetGameObjectDragAndDropSource(sourceTarget);
	SetGameObjectDragAndDropTarget(sourceTarget);
}

void PanelHierarchy::SetGameObjectDragAndDropSource(GameObject* source) const
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("GAMEOBJECTS_HIERARCHY", &source, sizeof(GameObject));
		ImGui::EndDragDropSource();
	}
}

void PanelHierarchy::SetGameObjectDragAndDropTarget(GameObject* target) const
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			GameObject* payload_n = *(GameObject**)payload->Data;

			payload_n->GetParent()->EraseChild(payload_n);
			target->AddChild(payload_n);

			payload_n->SetParent(target);
		}
		ImGui::EndDragDropTarget();
	}
}

#endif // GAME
