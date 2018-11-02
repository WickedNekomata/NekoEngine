#include "ComponentMaterial.h"
#include "GameObject.h"
#include "MaterialImporter.h"

#include "imgui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::Material_Component) {}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentType::Material_Component)
{
	textures = componentMaterial.textures;
}

ComponentMaterial::~ComponentMaterial()
{
	parent->materialRenderer = nullptr;
}

void ComponentMaterial::Update() {}

void ComponentMaterial::OnUniqueEditor()
{
	ImGui::Text("THIS IS A MATERIAL COMPONENT");

	/*
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
	*/
}

void ComponentMaterial::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "ResourceMaterial", 0012013);
}

void ComponentMaterial::OnLoad(JSON_Object* file)
{
	//LOAD MATERIAL
}