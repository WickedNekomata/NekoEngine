#include "PanelInspector.h"

#include "Globals.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "ImGui/imgui.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGui::SetNextWindowPos({ (float)App->window->GetWindowWidth() - 300,50 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300,(float)App->window->GetWindowHeight() - 50 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoSavedSettings;
	inspectorFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;

	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		const char* geometryName = App->renderer3D->GetGeometryName();
		if (geometryName != nullptr)
		{
			bool isGeometryActive = App->renderer3D->IsGeometryActive();
			if (ImGui::Checkbox(geometryName, &isGeometryActive))
				App->renderer3D->SetGeometryActive(isGeometryActive);
		}

		ImGui::Spacing();

		Mesh* mesh = App->renderer3D->GetMeshAt(0);

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (mesh != nullptr)
			{
				ImGui::Text("Position");
				math::float3 position = mesh->position;
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##posX", &position.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##posY", &position.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##posZ", &position.z);

				ImGui::Text("Rotation");
				math::Quat rotationQuat = mesh->rotation;
				math::float3 rotationEuler = RADTODEG * rotationQuat.ToEulerXYZ();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##rotX", &rotationEuler.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##rotY", &rotationEuler.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##rotZ", &rotationEuler.z);

				ImGui::Text("Scale");
				math::float3 scale = mesh->scale;
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##scaleX", &scale.x); ImGui::SameLine();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##scaleY", &scale.y); ImGui::SameLine();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::PushItemWidth(70);
				ImGui::InputFloat("##scaleZ", &scale.z);
			}
		}

		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen))
		{
			uint numMeshes = App->renderer3D->GetNumMeshes();
			ImGui::Text("Meshes: %i", numMeshes);

			for (int i = 0; i < numMeshes; ++i)
			{
				ImGui::Separator();
				Mesh* mesh = App->renderer3D->GetMeshAt(i);
				ImGui::TextColored(WHITE, "Mesh %i: %s", i + 1, mesh->name);
				ImGui::Separator();

				ImGui::Text("Vertices: %i", mesh->verticesSize);
				ImGui::Text("Vertices ID: %i", mesh->verticesID);

				ImGui::Text("Indices: %i", mesh->indicesSize);
				ImGui::Text("Indices ID: %i", mesh->indicesID);

				ImGui::Text("Texture Coords: %i", mesh->verticesSize);
				ImGui::Text("Texture Coords ID: %i", mesh->textureCoordsID);

				ImGui::Text("Triangles: %i", mesh->indicesSize / 3);
			}
		}

		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (mesh != nullptr)
			{
				uint numTextures = 0;

				bool checkTexture = App->renderer3D->IsCheckTexture();
				if (ImGui::Checkbox("Check Texture", &checkTexture)) { App->renderer3D->SetCheckTexture(checkTexture); }			

				bool multitexturing = App->renderer3D->GetMultitexturing();
				if (ImGui::Checkbox("Multitexturing", &multitexturing)) 
				{ 
					App->renderer3D->SetMultitexturing(multitexturing); 

					App->tex->SetDroppedTextureUnit(0);
					App->renderer3D->SetCurrentTextureUnits(1);

					if (!multitexturing)
					{
						for (uint i = 1; i < App->renderer3D->GetMaxTextureUnits(); ++i)
							App->renderer3D->AddTextureToRemove(i);
					}
				}

				if (multitexturing)
				{
					int currentTextureUnits = App->renderer3D->GetCurrentTextureUnits();
					if (ImGui::SliderInt("Texture units", &currentTextureUnits, 1, App->renderer3D->GetMaxTextureUnits() - 1)) 
					{ 
						App->tex->SetDroppedTextureUnit(0);
						App->renderer3D->SetCurrentTextureUnits(currentTextureUnits);

						for (uint i = currentTextureUnits; i < App->renderer3D->GetMaxTextureUnits(); ++i)
							App->renderer3D->AddTextureToRemove(i);
					}

					int droppedTextureUnit = App->tex->GetDroppedTextureUnit();
					int maxSliderInt = currentTextureUnits;
					if (maxSliderInt > 0)
						--maxSliderInt;
					if (ImGui::SliderInt("Dropped texture unit", &droppedTextureUnit, 0, maxSliderInt)) { App->tex->SetDroppedTextureUnit(droppedTextureUnit); }
				}

				for (uint i = 0; i < App->renderer3D->GetMaxTextureUnits(); ++i)
				{
					if (mesh->texturesID[i] > 0)
						++numTextures;
				}

				ImGui::Text("Textures: %i", numTextures);

				for (uint i = 0; i < App->renderer3D->GetCurrentTextureUnits(); ++i)
				{
					ImGui::Separator();
					ImGui::TextColored(WHITE, "Texture %i:", i);
					ImGui::Separator();

					ImGui::Image((void*)(intptr_t)mesh->texturesID[i], ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

					ImGui::Text("ID: %i", mesh->texturesID[i]);

					if (mesh->texturesID[i] != App->tex->GetCheckTextureID())
						ImGui::Text("%i x %i", mesh->texturesWidth[i], mesh->texturesHeight[i]);

					ImGui::PushID(i);
					if (ImGui::SmallButton("Remove texture")) { App->renderer3D->AddTextureToRemove(i); }
					ImGui::PopID();
				}
			}
		}
	}
	ImGui::End();

	return true;
}