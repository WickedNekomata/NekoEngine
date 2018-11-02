#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "Component.h"

#include "ImGui/imgui.h"
#include "imgui/imgui_internal.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoSavedSettings;
	inspectorFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	
	if (ImGui::Begin(name, &enabled))
	{
		if (App->scene->currentGameObject != nullptr)
		{
			GameObject* gameObject = App->scene->currentGameObject;

			bool isActive = gameObject->IsActive();
			if (ImGui::Checkbox("##Active", &isActive)) { gameObject->ToggleIsActive(); }

			ImGui::SameLine();
			static char objName[INPUT_BUF_SIZE];
			if (gameObject->GetName() != nullptr)
				strcpy_s(objName, IM_ARRAYSIZE(objName), gameObject->GetName());

			ImGui::PushItemWidth(100);
			ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
			if (ImGui::InputText("##objName", objName, IM_ARRAYSIZE(objName), inputFlag))
				gameObject->SetName(objName);
			ImGui::PopItemWidth();

			ImGui::SameLine(0.0f, 30.f);
			bool isStatic = gameObject->IsStatic();
			if (ImGui::Checkbox("##static", &isStatic)) { gameObject->ToggleIsStatic(); }
			ImGui::SameLine();
			ImGui::Text("Static");

			ImGui::Text("Tag");
			ImGui::SameLine();
			const char* tags[] = { "Untagged", "Player" };
			static int currentTag = 0;
			ImGui::PushItemWidth(75);
			ImGui::Combo("##tag", &currentTag, tags, IM_ARRAYSIZE(tags));
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::Text("Layer");
			ImGui::SameLine();
			const char* layers[] = { "Default", "Collider", "PostProcessing" };
			static int currentLayer = 0;
			ImGui::PushItemWidth(75);
			ImGui::Combo("##layer", &currentLayer, layers, IM_ARRAYSIZE(layers));
			ImGui::PopItemWidth();

			for (int i = 0; i < gameObject->GetComponenetsLength(); ++i)
			{
				ImGui::Separator();
				gameObject->GetComponent(i)->OnEditor();
			}
			ImGui::Separator();
			ImGui::Button("Add Component");
			if (ImGui::BeginPopupContextItem((const char*)0, 0))
			{
				if (gameObject->meshRenderer == nullptr) {
					if (ImGui::Selectable("Mesh")) {
						gameObject->AddComponent(ComponentType::Mesh_Component);
						ImGui::CloseCurrentPopup();
					}
				}
				if (gameObject->materialRenderer == nullptr) {
					if (ImGui::Selectable("Material")) {
						gameObject->AddComponent(ComponentType::Material_Component);
						ImGui::CloseCurrentPopup();
					}
				}
				if (gameObject->camera == nullptr)
					if (ImGui::Selectable("Camera")) {
						gameObject->AddComponent(ComponentType::Camera_Component);
						ImGui::CloseCurrentPopup();
					}
				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();

	return true;
}
