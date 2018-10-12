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

			ImGui::Checkbox("", &currObject->enabled);
			ImGui::SameLine();

			static char objName[STR_INPUT_SIZE];
			if (currObject->GetName() != nullptr)
				strcpy_s(objName, IM_ARRAYSIZE(objName), currObject->GetName());
			if (ImGui::InputText("", objName, IM_ARRAYSIZE(objName)))
				currObject->SetName(objName);

			/*
			ImGui::Text("Tag"); 
			ImGui::SameLine();
			const char* tags[] = { "Untagged", "Player" };
			static int currentTag = 0;
			ImGui::Combo("", &currentTag, tags, IM_ARRAYSIZE(tags));

			ImGui::Text("Layer");
			ImGui::SameLine();
			const char* layers[] = { "Default", "Collider", "PostProcessing" };
			static int currentLayer = 0;
			ImGui::Combo("", &currentLayer, layers, IM_ARRAYSIZE(layers));
			*/

			for (int i = 0; i < currObject->GetComponenetsLength(); ++i)
			{
				ImGui::Separator();
				currObject->GetComponent(i)->OnEditor();
			}
			ImGui::Separator();
			if (ImGui::Button("Add Component"))
			{

			}
		}
	}
	ImGui::End();
	return true;
}
