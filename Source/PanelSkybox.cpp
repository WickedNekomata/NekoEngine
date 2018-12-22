#include "PanelSkybox.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "MaterialImporter.h"
#include "ResourceTexture.h"

#include "imgui/imgui.h"

PanelSkybox::PanelSkybox(char* name) : Panel(name)
{
}

PanelSkybox::~PanelSkybox()
{
}

bool PanelSkybox::Draw()
{
	ImGui::Begin(name, &enabled);

	for (int i = 0; i < App->renderer3D->cubemapTextures.size(); ++i)
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
		ResourceTexture* tex = (ResourceTexture*)App->res->GetResource(App->renderer3D->cubemapTextures[i]);
		if (tex != NULL) {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##%i", tex->GetName(), i);
			ImGui::Button(itemName, ImVec2(100.0f, 0.0f));
		}
		else {
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Replace Me!##%i", i);
			ImGui::Button(itemName, ImVec2(100.0f, 0.0f));
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				if (tex != nullptr)
					App->res->SetAsUnused(tex->GetUUID());
				uint payload_n = *(uint*)payload->Data;
				ResourceTexture* res = (ResourceTexture*)App->res->GetResource(payload_n);
				if (res != nullptr)
				{
					App->renderer3D->cubemapTextures[i] = res->GetUUID();
					App->res->SetAsUsed(App->renderer3D->cubemapTextures[i]);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Separator();

	if (ImGui::Button("Set new Skybox")) {
		App->materialImporter->DeleteTexture(App->renderer3D->cubemapTexture);
		std::vector<uint> texturesToLoad;
		texturesToLoad.reserve(6);
		for (int i = 0; i < 6; ++i) {
			ResourceTexture* res = (ResourceTexture*)App->res->GetResource(App->renderer3D->cubemapTextures[i]);
			if (res != nullptr)
				texturesToLoad.push_back(res->id);
			else
				texturesToLoad.push_back(0);
		}
		texturesToLoad.shrink_to_fit();
		App->renderer3D->cubemapTexture = App->materialImporter->LoadCubemapTexture(texturesToLoad);
	}

	ImGui::End();

	return true;
}

#endif