#include "PanelSkybox.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "MaterialImporter.h"
#include "ResourceTexture.h"

#include "imgui/imgui.h"

PanelSkybox::PanelSkybox(char* name) : Panel(name)
{
	cubemapTextures.reserve(6);
	for (int i = 0; i < 6; ++i)
		cubemapTextures.push_back(nullptr);
	cubemapTextures.shrink_to_fit();
}

PanelSkybox::~PanelSkybox()
{
}

bool PanelSkybox::Draw()
{
	ImGui::Begin(name, &enabled);

	for (int i = 0; i < cubemapTextures.size(); ++i)
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
		ResourceTexture* tex = cubemapTextures[i];
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
				cubemapTextures[i] = res;
				if (res != nullptr)
					App->res->SetAsUsed(cubemapTextures[i]->GetUUID());
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
			if (cubemapTextures[i] != nullptr)
				texturesToLoad.push_back(cubemapTextures[i]->id);
			else
				texturesToLoad.push_back(0);
		}
		texturesToLoad.shrink_to_fit();
		App->renderer3D->cubemapTexture = App->materialImporter->LoadCubemapTexture(texturesToLoad);
	}

	ImGui::End();

	return true;
}

void PanelSkybox::ClearSkybox()
{
	for (int i = 0; i < cubemapTextures.size(); ++i) {
		if (cubemapTextures[i] != nullptr)
			App->res->SetAsUnused(cubemapTextures[i]->GetUUID());
	}

	cubemapTextures.clear();
}
