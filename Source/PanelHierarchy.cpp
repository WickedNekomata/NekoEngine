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
				if (ImGui::TreeNodeEx(child->GetName(), treeNodeFlags))
				{
					// TODO: Bug with contextitem and isitemhovered(). only workig if treenode is open.
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::Selectable("Create object")) {
							App->GOs->CreateGameObject((char*)child->GetName(), child);
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("Delete")) {
							MessageBox(0, "CHECK WARNING TODO at Gos's PostUdate Method", "MessageBox caption", MB_OK);
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
						App->scene->currentGameObject = child;
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
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::Selectable("Create object")) {
						App->GOs->CreateGameObject("aaa", child);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("Delete")) {
						MessageBox(0, "CHECK WARNING TODO at Gos's PostUdate Method", "MessageBox caption", MB_OK);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered())
					App->scene->currentGameObject = child;
			}
		}
	}
}
