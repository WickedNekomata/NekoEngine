#include "PanelSimulatedTime.h"

#ifndef GAMEMODE
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleGui.h"
#include "ComponentEmitter.h"
#include "ModuleScene.h"

#include "ImGui\imgui.h"

PanelSimulatedTime::PanelSimulatedTime(char* name) : Panel(name) {}

PanelSimulatedTime::~PanelSimulatedTime() {}

bool PanelSimulatedTime::Draw()
{
	GameObject* selectedGO = App->scene->selectedObject.GetCurrGameObject();
	if (selectedGO)
	{
		ComponentEmitter* emitter = (ComponentEmitter*)selectedGO->GetComponentByType(EmitterComponent);
		if (emitter && !emitter->isSubEmitter)
		{
			if (ImGui::Begin(name, &enabled, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
			{

				if (ImGui::Button("Play", { 50,25 }))
				{
					if (emitter->simulatedGame == SimulatedGame_PAUSE)
						emitter->timeSimulating.Continue();
					else
						emitter->timeSimulating.Start();

					emitter->ChangeGameState(SimulatedGame_PLAY);
					emitter->StartEmitter();
				}

				ImGui::SameLine();

				if (ImGui::Button("Pause", { 50,25 }))
				{
					emitter->ChangeGameState(SimulatedGame_PAUSE);
					emitter->timeSimulating.Pause();
				}
				ImGui::SameLine();

				if (ImGui::Button("Stop", { 50,25 }))
				{
					emitter->ChangeGameState(SimulatedGame_STOP);
					emitter->timeSimulating.Stop();
				}

				ImGui::Text("Game timer: ");

				ImGui::SameLine();
				ImGui::Text("%.3f", emitter->timeSimulating.ReadSec());

			}
			ImGui::End();
		}
	}
	return true;
}
#endif