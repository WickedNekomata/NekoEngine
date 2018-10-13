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

				if (mesh->textureID > 0)
					++numTextures;

				bool multitexturing = App->renderer3D->GetMultitexturing();
				static int currentTexture = 0;
				if (ImGui::Checkbox("Multitexturing", &multitexturing))
				{
					if (multitexturing)
					{
						App->renderer3D->SetMultitexturing(true);

						if (currentTexture == 0)
							App->renderer3D->AddTexture2ToMeshes(App->tex->GetMultitexturingTextureID());
						else if (currentTexture == 1)
							App->renderer3D->AddTexture2ToMeshes(App->tex->GetMultitexturingTexture2ID());
					}
					else
					{
						App->renderer3D->SetMultitexturing(false);

						App->renderer3D->AddTexture2ToMeshes(0);
					}
				}

				if (multitexturing)
					++numTextures;

				ImGui::Text("Textures: %i", numTextures);

				if (mesh->textureID > 0)
				{
					ImGui::Separator();
					ImGui::TextColored(WHITE, "Texture 1:");
					ImGui::Separator();

					ImGui::Image((void*)(intptr_t)mesh->textureID, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
					
					ImGui::Text("ID: %i", mesh->textureID);

					if (mesh->textureID != App->tex->GetCheckTextureID())
					{
						ImGui::Text("Width: %i", mesh->textureWidth);
						ImGui::Text("Height %i", mesh->textureHeight);
					}
				}	

				if (mesh->texture2ID > 0)
				{
					ImGui::Separator();
					ImGui::TextColored(WHITE, "Texture 2:");
					ImGui::Separator();

					const char* multitexturingTextures[] = { "Smile", "Mask" };
					if (ImGui::Combo("##texture2", &currentTexture, multitexturingTextures, IM_ARRAYSIZE(multitexturingTextures)))
					{
						if (currentTexture == 0)
							App->renderer3D->AddTexture2ToMeshes(App->tex->GetMultitexturingTextureID());
						else if (currentTexture == 1)
							App->renderer3D->AddTexture2ToMeshes(App->tex->GetMultitexturingTexture2ID());
					}

					ImGui::Image((void*)(intptr_t)mesh->texture2ID, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

					ImGui::Text("ID: %i", mesh->texture2ID);
				}
			}
		}
	}
	ImGui::End();

	return true;
}