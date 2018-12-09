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
		ImGui::Text("texture %i", i + 1);

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
					App->res->SetAsUnused(tex->id);
				ResourceTexture* payload_n = (ResourceTexture*)payload->Data;
				cubemapTextures[i] = payload_n;
				if (payload_n != nullptr)
					App->res->SetAsUsed(cubemapTextures[i]->id);
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
				texturesToLoad.push_back(App->materialImporter->GetDefaultTexture());
		}
		texturesToLoad.shrink_to_fit();
		App->renderer3D->cubemapTexture = App->materialImporter->LoadCubemapTexture(texturesToLoad);
	}

	ImGui::End();

	return true;
}
