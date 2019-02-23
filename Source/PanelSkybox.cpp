#include "PanelSkybox.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "MaterialImporter.h"
#include "ResourceTexture.h"

#include "imgui/imgui.h"

PanelSkybox::PanelSkybox(const char* name) : Panel(name) {}

PanelSkybox::~PanelSkybox() {}

bool PanelSkybox::Draw()
{
	ImGui::Begin(name, &enabled);

	for (uint i = 0; i < App->renderer3D->skyboxTextures.size(); ++i)
	{
		switch (i)
		{
		case 0:
			ImGui::Text("Right");
			break;
		case 1:
			ImGui::Text("Left");
			break;
		case 2:
			ImGui::Text("Bottom");
			break;
		case 3:
			ImGui::Text("Top");
			break;
		case 4:
			ImGui::Text("Front");
			break;
		case 5:
			ImGui::Text("Back");
			break;
		}

		char itemName[DEFAULT_BUF_SIZE];
		ResourceTexture* tex = (ResourceTexture*)App->res->GetResource(App->renderer3D->skyboxTextures[i]);
		if (tex != NULL) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##%i", tex->GetName(), i);
			ImGui::Button(itemName, ImVec2(100.0f, 0.0f));
		}
		else {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "No Texture##%i", i);
			ImGui::Button(itemName, ImVec2(100.0f, 0.0f));
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				if (tex != nullptr)
					App->res->SetAsUnused(tex->GetUuid());

				uint payload_n = *(uint*)payload->Data;
				ResourceTexture* res = (ResourceTexture*)App->res->GetResource(payload_n);
				if (res != nullptr)
				{
					App->renderer3D->skyboxTextures[i] = res->GetUuid();
					App->res->SetAsUsed(App->renderer3D->skyboxTextures[i]);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("SET NEW SKYBOX")) 
	{
		if (App->materialImporter->GetSkyboxTexture() != App->renderer3D->skyboxTexture)
			App->materialImporter->DeleteTexture(App->renderer3D->skyboxTexture);

		std::vector<uint> texturesToLoad;
		texturesToLoad.reserve(6);
		for (uint i = 0; i < 6; ++i) 
		{
			ResourceTexture* res = (ResourceTexture*)App->res->GetResource(App->renderer3D->skyboxTextures[i]);
			if (res != nullptr)
				texturesToLoad.push_back(res->GetId());
			else
				texturesToLoad.push_back(0);
		}

		App->renderer3D->skyboxTexture = App->materialImporter->LoadCubemapTexture(texturesToLoad);
	}
	else if (ImGui::Button("USE DEFAULT SKYBOX"))
	{
		App->renderer3D->ClearSkybox();

		if (App->materialImporter->GetSkyboxTexture() != App->renderer3D->skyboxTexture)
		{
			App->materialImporter->DeleteTexture(App->renderer3D->skyboxTexture);
			App->renderer3D->skyboxTexture = App->materialImporter->GetSkyboxTexture();
		}

		App->renderer3D->skyboxTextures = App->materialImporter->GetSkyboxTextures();
	}
	ImGui::End();

	return true;
}

#endif