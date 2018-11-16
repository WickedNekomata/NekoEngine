#include "PanelInspector.h"

#ifndef GAMEMODE

#include "Globals.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"

#include "GameObject.h"
#include "Component.h"

#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"

#include "ImGui/imgui.h"
#include "imgui/imgui_internal.h"

PanelInspector::PanelInspector(char* name) : Panel(name) {}

PanelInspector::~PanelInspector() {}

bool PanelInspector::Draw()
{
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		switch (App->scene->selectedObject.GetType())
		{
		case CurrentSelection::SelectedType::gameObject:
			ShowGameObjectInspector();
			break;
		case CurrentSelection::SelectedType::scene:
			ShowSceneInspector();
			break;
		case CurrentSelection::SelectedType::resource:
		{
			if (((Resource*)App->scene->selectedObject.Get())->GetType() == ResourceType::Mesh_Resource)
				ShowMeshResourceInspector();
			else
				ShowTextureResourceInspector();
			break;
		}
		case CurrentSelection::SelectedType::meshImportSettings:
			ShowMeshImportSettingsInspector();
			break;
		case CurrentSelection::SelectedType::textureImportSettings:
			ShowTextureImportSettingsInspector();
			break;
		}
	}
	ImGui::End();
	
	return true;
}

void PanelInspector::ShowGameObjectInspector()
{
	GameObject* gameObject = (GameObject*)App->scene->selectedObject.Get();

	if (gameObject == nullptr)
	{
		assert(gameObject != nullptr);
		return;
	}

	bool isActive = gameObject->IsActive();
	if (ImGui::Checkbox("##Active", &isActive)) { gameObject->ToggleIsActive(); }

	ImGui::SameLine();
	static char objName[INPUT_BUF_SIZE];
	if (gameObject->GetName() != nullptr)
		strcpy_s(objName, IM_ARRAYSIZE(objName), gameObject->GetName());

	ImGui::PushItemWidth(100.0f);
	ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputText("##objName", objName, IM_ARRAYSIZE(objName), inputFlag))
		gameObject->SetName(objName);
	ImGui::PopItemWidth();

	ImGui::SameLine(0.0f, 30.f);
	bool isStatic = gameObject->IsStatic();
	if (ImGui::Checkbox("##static", &isStatic)) { gameObject->ToggleIsStatic(); }
	ImGui::SameLine();
	ImGui::Text("Static");

	for (int i = 0; i < gameObject->GetComponenetsLength(); ++i)
	{
		ImGui::Separator();
		DragnDropSeparatorTarget(gameObject->GetComponent(i));
		gameObject->GetComponent(i)->OnEditor();
	}
	ImGui::Separator();
	DragnDropSeparatorTarget(gameObject->GetComponent(gameObject->GetComponenetsLength() - 1));

	ImGui::Button("Add Component");
	if (ImGui::BeginPopupContextItem((const char*)0, 0))
	{
		if (gameObject->meshRenderer == nullptr) {
			if (ImGui::Selectable("Mesh")) {
				gameObject->AddComponent(ComponentType::Mesh_Component);
				ImGui::CloseCurrentPopup();
			}
		}
		if (gameObject->materialRenderer == nullptr) {
			if (ImGui::Selectable("Material")) {
				gameObject->AddComponent(ComponentType::Material_Component);
				ImGui::CloseCurrentPopup();
			}
		}
		if (gameObject->camera == nullptr)
			if (ImGui::Selectable("Camera")) {
				gameObject->AddComponent(ComponentType::Camera_Component);
				ImGui::CloseCurrentPopup();
			}
		ImGui::EndPopup();		
	}
}

void PanelInspector::DragnDropSeparatorTarget(Component* target)
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMPONENTS_INSPECTOR"))
		{
			Component* payload_n = *(Component**)payload->Data;
			target->GetParent()->ReorderComponents(payload_n, target);
		}
		ImGui::EndDragDropTarget();
	}
}

void PanelInspector::ShowSceneInspector() const
{
	ImGui::Text("Scene");
}

void PanelInspector::ShowMeshResourceInspector() const
{
	ImGui::Text("Mesh Resource");
	ImGui::Separator();
	ImGui::Spacing();

	const ResourceMesh* resourceMesh = (const ResourceMesh*)App->scene->selectedObject.Get();
	ImGui::Text("File: %s", resourceMesh->file.data());
	ImGui::Text("Exported file: %s", resourceMesh->exportedFile.data());
	ImGui::Text("UUID: %u", resourceMesh->GetUUID());
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool inMemory = resourceMesh->IsInMemory();
	ImGui::Checkbox("In memory", &inMemory);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
	if (inMemory)
		ImGui::Text("References: %u", resourceMesh->CountReferences());

	ImGui::Text("Vertices: %i", resourceMesh->verticesSize);
	ImGui::Text("Vertices ID: %i", resourceMesh->verticesID);

	ImGui::Text("Indices: %i", resourceMesh->indicesSize);
	ImGui::Text("Indices ID: %i", resourceMesh->indicesID);

	ImGui::Text("Triangles: %i", resourceMesh->indicesSize / 3);

	ImGui::Text("Texture Coords: %i", resourceMesh->verticesSize);
	ImGui::Text("Texture Coords ID: %i", resourceMesh->textureCoordsID);
}

void PanelInspector::ShowTextureResourceInspector() const
{
	ImGui::Text("Texture Resource");
	ImGui::Separator();
	ImGui::Spacing();

	const ResourceTexture* resourceTexture = (const ResourceTexture*)App->scene->selectedObject.Get();
	ImGui::Text("File: %s", resourceTexture->file.data());
	ImGui::Text("Exported file: %s", resourceTexture->exportedFile.data());
	ImGui::Text("UUID: %u", resourceTexture->GetUUID());

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	bool inMemory = resourceTexture->IsInMemory();
	ImGui::Checkbox("In memory", &inMemory);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
	if (inMemory)
		ImGui::Text("References: %u", resourceTexture->CountReferences());

	ImGui::Text("ID: %u", resourceTexture->id);
	ImGui::Image((void*)(intptr_t)resourceTexture->id, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Text("%u x %u", resourceTexture->width, resourceTexture->height);
}

void PanelInspector::ShowMeshImportSettingsInspector()
{
	MeshImportSettings* meshImportSettings = (MeshImportSettings*)App->scene->selectedObject.Get();

	ImGui::Text("Mesh Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	const char* postProcessConfiguration[] = { "Target Realtime Fast", "Target Realtime Quality", "Target Realtime Max Quality", "Custom" };
	static int currentPostProcessConfiguration = meshImportSettings->postProcessConfiguration;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Configuration", &currentPostProcessConfiguration, postProcessConfiguration, IM_ARRAYSIZE(postProcessConfiguration)))
		meshImportSettings->postProcessConfiguration = (MeshImportSettings::MeshPostProcessConfiguration)currentPostProcessConfiguration;
	ImGui::PopItemWidth();

	bool calcTangentSpace = meshImportSettings->calcTangentSpace;
	bool genNormals = meshImportSettings->genNormals;
	bool genSmoothNormals = meshImportSettings->genSmoothNormals;
	bool joinIdenticalVertices = meshImportSettings->joinIdenticalVertices;
	bool triangulate = meshImportSettings->triangulate;
	bool genUVCoords = meshImportSettings->genUVCoords;
	bool sortByPType = meshImportSettings->sortByPType;
	bool improveCacheLocality = meshImportSettings->improveCacheLocality;
	bool limitBoneWeights = meshImportSettings->limitBoneWeights;
	bool removeRedundantMaterials = meshImportSettings->removeRedundantMaterials;
	bool splitLargeMeshes = meshImportSettings->splitLargeMeshes;
	bool findDegenerates = meshImportSettings->findDegenerates;
	bool findInvalidData = meshImportSettings->findInvalidData;
	bool findInstances = meshImportSettings->findInstances;
	bool validateDataStructure = meshImportSettings->validateDataStructure;
	bool optimizeMeshes = meshImportSettings->optimizeMeshes;

	switch (currentPostProcessConfiguration)
	{
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_FAST:
		calcTangentSpace = true;
		genNormals = true;
		genSmoothNormals = false;
		joinIdenticalVertices = true;
		triangulate = true;
		genUVCoords = true;
		sortByPType = true;
		improveCacheLocality = false;
		limitBoneWeights = false;
		removeRedundantMaterials = false;
		splitLargeMeshes = false;
		findDegenerates = false;
		findInvalidData = false;
		findInstances = false;
		validateDataStructure = false;
		optimizeMeshes = false;
		break;
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_QUALITY:
		calcTangentSpace = true;
		genNormals = false;
		genSmoothNormals = true;
		joinIdenticalVertices = true;
		triangulate = true;
		genUVCoords = true;
		sortByPType = true;
		improveCacheLocality = true;
		limitBoneWeights = true;
		removeRedundantMaterials = true;
		splitLargeMeshes = true;
		findDegenerates = true;
		findInvalidData = true;
		findInstances = false;
		validateDataStructure = false;
		optimizeMeshes = false;
		break;
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_MAX_QUALITY:
		calcTangentSpace = true;
		genNormals = false;
		genSmoothNormals = true;
		joinIdenticalVertices = true;
		triangulate = true;
		genUVCoords = true;
		sortByPType = true;
		improveCacheLocality = true;
		limitBoneWeights = true;
		removeRedundantMaterials = true;
		splitLargeMeshes = true;
		findDegenerates = true;
		findInvalidData = true;
		findInstances = true;
		validateDataStructure = true;
		optimizeMeshes = true;
		break;
	}

	if (currentPostProcessConfiguration != MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

	if (ImGui::Checkbox("Calculate Tangent Space", &calcTangentSpace))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->calcTangentSpace = calcTangentSpace;
	}
	if (ImGui::Checkbox("Generate Normals", &genNormals))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->genNormals = genNormals;
	}
	if (ImGui::Checkbox("Generate Smooth Normals", &genSmoothNormals))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->genSmoothNormals = genSmoothNormals;
	}
	if (ImGui::Checkbox("Join Identical Vertices", &joinIdenticalVertices))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->joinIdenticalVertices = joinIdenticalVertices;
	}
	if (ImGui::Checkbox("Triangulate", &triangulate))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->triangulate = triangulate;
	}
	if (ImGui::Checkbox("Generate UV Coordinates", &genUVCoords))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->genUVCoords = genUVCoords;
	}
	if (ImGui::Checkbox("Sort By Primitive Type", &sortByPType))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->sortByPType = sortByPType;
	}
	if (ImGui::Checkbox("Improve Cache Locality", &improveCacheLocality))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->improveCacheLocality = improveCacheLocality;
	}
	if (ImGui::Checkbox("Limit Bone Weights", &limitBoneWeights))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->limitBoneWeights = limitBoneWeights;
	}
	if (ImGui::Checkbox("Remove Redundant Materials", &removeRedundantMaterials))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->removeRedundantMaterials = removeRedundantMaterials;
	}
	if (ImGui::Checkbox("Split Large Meshes", &splitLargeMeshes))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->splitLargeMeshes = splitLargeMeshes;
	}
	if (ImGui::Checkbox("Find Degenerates", &findDegenerates))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->findDegenerates = findDegenerates;
	}
	if (ImGui::Checkbox("Find Invalid Data", &findInvalidData))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->findInvalidData = findInvalidData;
	}
	if (ImGui::Checkbox("Find Instances", &findInstances))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->findInstances = findInstances;
	}
	if (ImGui::Checkbox("Validate Data Structure", &validateDataStructure))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->validateDataStructure = validateDataStructure;
	}
	if (ImGui::Checkbox("Optimize Meshes", &optimizeMeshes))
	{
		if (currentPostProcessConfiguration == MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
			meshImportSettings->optimizeMeshes = optimizeMeshes;
	}

	if (currentPostProcessConfiguration != MeshImportSettings::MeshPostProcessConfiguration::CUSTOM)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

	ImGui::Spacing();
	if (ImGui::Button("REIMPORT")) 
	{ 
		App->sceneImporter->SetMeshImportSettingsToMeta(meshImportSettings->metaFile, meshImportSettings);

		System_Event newEvent;
		newEvent.fileEvent.metaFile = meshImportSettings->metaFile;
		newEvent.type = System_Event_Type::ReimportFile;
		App->PushSystemEvent(newEvent);
	}
}

void PanelInspector::ShowTextureImportSettingsInspector()
{
	TextureImportSettings* textureImportSettings = (TextureImportSettings*)App->scene->selectedObject.Get();

	ImGui::Text("Texture Import Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Import Settings");
	ImGui::Spacing();

	const char* compression[] = { "DXT1", "DXT2", "DXT3", "DXT4", "DXT5" };
	int currentCompression = textureImportSettings->compression;
	ImGui::PushItemWidth(100.0f);
	if (ImGui::Combo("Compression", &currentCompression, compression, IM_ARRAYSIZE(compression)))
		textureImportSettings->compression = (TextureImportSettings::TextureCompression)currentCompression;

	ImGui::Spacing();
	ImGui::Text("Load Settings");
	ImGui::Spacing();

	ImGui::Text("Wrap Mode");
	const char* wrap[] = { "Repeat", "Mirrored Repeat", "Clamp To Edge", "Clamp To Border" };
	int currentWrapS = textureImportSettings->wrapS;
	int currentWrapT = textureImportSettings->wrapT;
	if (ImGui::Combo("Wrap S", &currentWrapS, wrap, IM_ARRAYSIZE(wrap)))
		textureImportSettings->wrapS = (TextureImportSettings::TextureWrapMode)currentWrapS;
	if (ImGui::Combo("Wrap T", &currentWrapT, wrap, IM_ARRAYSIZE(wrap)))
		textureImportSettings->wrapT = (TextureImportSettings::TextureWrapMode)currentWrapT;

	ImGui::Text("Filter Mode");
	const char* filter[] = { "Nearest", "Linear",
		"Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear" };
	int currentMinFilter = textureImportSettings->minFilter;
	int currentMagFilter = textureImportSettings->magFilter;
	if (ImGui::Combo("Min Filter", &currentMinFilter, filter, IM_ARRAYSIZE(filter)))
		textureImportSettings->minFilter = (TextureImportSettings::TextureFilterMode)currentMinFilter;
	if (ImGui::Combo("Mag Filter", &currentMagFilter, filter, IM_ARRAYSIZE(filter)))
		textureImportSettings->magFilter = (TextureImportSettings::TextureFilterMode)currentMagFilter;
	ImGui::PopItemWidth();

	if (textureImportSettings->UseMipmap())
		ImGui::TextColored(YELLOW, "Mip Maps will be generated");

	if (App->materialImporter->IsAnisotropySupported())
		ImGui::SliderFloat("Anisotropy", &textureImportSettings->anisotropy, 0.0f, App->materialImporter->GetLargestSupportedAnisotropy());

	ImGui::Spacing();
	if (ImGui::Button("REIMPORT")) 
	{ 
		App->materialImporter->SetTextureImportSettingsToMeta(textureImportSettings->metaFile, textureImportSettings); 
		
		System_Event newEvent;
		newEvent.fileEvent.metaFile = textureImportSettings->metaFile;
		newEvent.type = System_Event_Type::ReimportFile;
		App->PushSystemEvent(newEvent);
	}
}

#endif // GAME