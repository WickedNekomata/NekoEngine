#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "Component.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGui::SetNextWindowSize({ 250,300 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoSavedSettings;
	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		if (App->scene->currentGameObject != nullptr)
		{
			GameObject* currObject = App->scene->currentGameObject;

			ImGui::Checkbox("##Enabled", &currObject->enabled);

			ImGui::SameLine();
			static char objName[INPUT_BUF_SIZE];
			if (currObject->GetName() != nullptr)
				strcpy_s(objName, IM_ARRAYSIZE(objName), currObject->GetName());
			ImGui::PushItemWidth(100);
			if (ImGui::InputText("##objName", objName, IM_ARRAYSIZE(objName)))
				currObject->SetName(objName);
			ImGui::PopItemWidth();

			ImGui::SameLine(0.0f, 30.f);
			static bool isStatic = false;
			ImGui::Checkbox("##static", &isStatic);
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

			for (int i = 0; i < currObject->GetComponenetsLength(); ++i)
			{
				ImGui::Separator();
				currObject->GetComponent(i)->OnEditor();
			}
			ImGui::Separator();
			ImGui::Button("Add Component");
			if (ImGui::BeginPopupContextItem((const char*)0, 0))
			{
				if (ImGui::Selectable("Mesh"))		{ currObject->AddComponent(ComponentType::Mesh_Component); 
													  ImGui::CloseCurrentPopup(); }
				if (ImGui::Selectable("Material"))  { currObject->AddComponent(ComponentType::Material_Component); 
													  ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
	return true;
}
