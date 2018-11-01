#include "PanelHierarchy.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ImGui/imgui.h"

#include "ComponentTransform.h"

PanelHierarchy::PanelHierarchy(char* name) : Panel(name) {}

PanelHierarchy::~PanelHierarchy() {}

bool PanelHierarchy::Draw()
{
	ImGui::SetNextWindowPos({ 0,(float)App->window->GetWindowHeight() - 200 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ (float)App->window->GetWindowWidth() - 300,200 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags hierarchyFlags = 0;
	hierarchyFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	hierarchyFlags |= ImGuiWindowFlags_NoSavedSettings;
	hierarchyFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;

	if (ImGui::Begin(name, &enabled, hierarchyFlags))
	{
		static char sceneName[INPUT_BUF_SIZE];
		strcpy_s(sceneName, IM_ARRAYSIZE(sceneName), App->GOs->nameScene);
		ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushItemWidth(100.0f);
		if (ImGui::InputText("##sceneName", sceneName, IM_ARRAYSIZE(sceneName), inputFlag))
			strcpy_s(App->GOs->nameScene, DEFAULT_BUF_SIZE, sceneName);

		ImGui::Separator();

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
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
				{
					GameObject* payload_n = *(GameObject**)payload->Data;

					payload_n->GetParent()->EraseChild(payload_n);
					root->AddChild(payload_n);

					payload_n->SetParent(root);
				}

				ImGui::EndDragDropTarget();
			}

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
			// TODO: Create a gameobject with the name of the parent + a number
			App->GOs->CreateGameObject("aaa", child);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("Delete")) {
			if (child == App->scene->currentGameObject)
				App->scene->currentGameObject = nullptr;
			App->GOs->DeleteGameObject(child);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void PanelHierarchy::SetGameObjectDragAndDrop(GameObject* SourceTarget)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("GAMEOBJECTS_HIERARCHY", &SourceTarget, sizeof(GameObject));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY"))
		{
			GameObject* payload_n = *(GameObject**)payload->Data;
			
			payload_n->GetParent()->EraseChild(payload_n);
			SourceTarget->AddChild(payload_n);
			
			payload_n->SetParent(SourceTarget);
		}
		ImGui::EndDragDropTarget();
	}
}
