#include "Application.h"
#include "ModuleImGui.h"
#include "ComponentEmitter.h"

void ModuleImGui::CreateGameTime()
{
	if (App->time->gameState == GameState_NONE)
	{
		ComponentEmitter* emitter = (ComponentEmitter*)App->geometry->currentGameObject->GetComponent(ComponentType_EMITTER);
		if (!emitter->isSubEmitter)
		{
			if (ImGui::Begin("GameSimulate", &gameTimeWindow, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
			{

				if (ImGui::Button("Play", { 50,25 }))
				{
					emitter->emitterActive = true;
					if (emitter->simulatedGame == GameState_PAUSE)
						emitter->timeSimulating.Continue();
					else
						emitter->timeSimulating.Start();

					emitter->ChangeGameState(GameState_PLAYING);
					emitter->StartEmitter();
				}

				ImGui::SameLine();

				if (ImGui::Button("Pause", { 50,25 }))
				{
					emitter->ChangeGameState(GameState_PAUSE);
					emitter->timeSimulating.Pause();
				}
				ImGui::SameLine();

				if (ImGui::Button("Stop", { 50,25 }))
				{
					emitter->ChangeGameState(GameState_STOP);
					emitter->timeSimulating.Stop();
				}

				ImGui::Text("Game timer: ");

				ImGui::SameLine();
				ImGui::Text("%.3f", emitter->timeSimulating.ReadSec());

			}
			ImGui::End();
		}
	}
}
