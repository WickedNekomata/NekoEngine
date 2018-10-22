#include "PanelHierarchy.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ImGui/imgui.h"

PanelHierarchy::PanelHierarchy(char* name) : Panel(name) {}

PanelHierarchy::~PanelHierarchy()
{
}

bool PanelHierarchy::Draw()
{
	ImGui::SetNextWindowSize({ 150,200 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags hierarchyFlags = 0;
	hierarchyFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	hierarchyFlags |= ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin(name, &enabled, hierarchyFlags))
	{
		GameObject* root = App->scene->root;

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Create object")) {
				App->GOs->CreateGameObject("patata", root);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::TreeNodeEx(root->GetName()))
		{
			IterateAllChildren(root);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelHierarchy::IterateAllChildren(GameObject* root)
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;

	if (root->HasChildren())
	{
		for (int i = 0; i < root->GetChildrenLength(); ++i)
		{
			GameObject* child = root->GetChild(i);

			if (child->HasChildren())
			{
				bool treeNodeOpened = false;
				if (ImGui::TreeNodeEx(child->GetName(), treeNodeFlags))
					treeNodeOpened = true;

				SetGameObjectDragAndDrop(child);
				AtGameObjectPopUp(child);

				if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
					App->scene->currentGameObject = child;

				if (treeNodeOpened) {
					IterateAllChildren(child);
					ImGui::TreePop();
				}
			}
			else
			{
				treeNodeFlags = 0;
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

				ImGui::TreeNodeEx(child->GetName(), treeNodeFlags);
				ImGui::TreePop();

				SetGameObjectDragAndDrop(child);
				AtGameObjectPopUp(child);

				if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
					App->scene->currentGameObject = child;
			}
		}
	}
}

void PanelHierarchy::AtGameObjectPopUp(GameObject* child)
{
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Create object")) {
			App->GOs->CreateGameObject("aaa", child);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("Delete")) {
			App->GOs->DeleteGameObject(child);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::SetGameObjectDragAndDrop(GameObject* root)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("GAMEOBJECTS_HIERARCHY", root, sizeof(GameObject));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			GameObject* payload_n = (GameObject*)payload->Data;
			
			//payload_n->GetParent()->EraseChild(payload_n);
			//root->AddChild(payload_n);
			
			// TODO: SWAP PARENTS AND PARENTS CHILD AND RECALCULATE EVERYTHING
		}
		ImGui::EndDragDropTarget();
	}
}
