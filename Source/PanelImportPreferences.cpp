#include "PanelImportPreferences.h"

#ifndef GAMEMODE

#include "Application.h"
#include "MaterialImporter.h"
#include "ComponentTransform.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

PanelImportPreferences::PanelImportPreferences(char* name) : Panel(name) {}

PanelImportPreferences::~PanelImportPreferences() {}

bool PanelImportPreferences::Draw()
{
	ImGuiWindowFlags importPreferencesFlags = 0;
	importPreferencesFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, importPreferencesFlags))
	{
		if (ImGui::TreeNode("Model"))
		{
			ModelNode();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Texture"))
		{
			TextureNode();
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelImportPreferences::ModelNode() const
{
	ImGui::Text("Import Options");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Scale");

	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

	static math::float3 scale = math::float3::one;
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleX", ImGuiDataType_Float, (void*)&scale.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleY", ImGuiDataType_Float, (void*)&scale.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	ImGui::PushItemWidth(TRANSFORMINPUTSWIDTH);
	ImGui::DragScalar("##ScaleZ", ImGuiDataType_Float, (void*)&scale.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	bool useFileScale = true;
	ImGui::Checkbox("Use File Scale", &useFileScale);

	// TODO: if useFileScale, show the scale of the file

	ImGui::Spacing();
	ImGui::Text("Post Process");

	const char* configuration[] = { "Target Realtime Fast", "Target Realtime Quality", "Target Realtime Max Quality", "Custom" };
	static int currentConfiguration = 0;
	ImGui::PushItemWidth(200.0f);
	ImGui::Combo("Configuration", &currentConfiguration, configuration, IM_ARRAYSIZE(configuration));
	ImGui::PopItemWidth();

	if (currentConfiguration < 3)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

	bool calcTangentSpace = true;
	ImGui::Checkbox("Calculate Tangent Space", &calcTangentSpace);

	if (currentConfiguration == 0 || currentConfiguration == 3)
	{
		bool genNormals = true;
		ImGui::Checkbox("Generate Normals", &genNormals);
	}

	if (currentConfiguration > 0)
	{
		// TODO CUSTOM ONLY CHECK ONE
		bool genSmoothNormals = true;
		ImGui::Checkbox("Generate Smooth Normals", &genSmoothNormals);
	}

	bool joinIdenticalVertices = true;
	ImGui::Checkbox("Join Identical Vertices", &joinIdenticalVertices);
	bool triangulate = true;
	ImGui::Checkbox("Triangulate", &triangulate);
	bool genUVCoords = true;
	ImGui::Checkbox("Generate UV Coordinates", &genUVCoords);
	bool sortByPType = true;
	ImGui::Checkbox("Sort By Primitive Type", &sortByPType);

	if (currentConfiguration > 0)
	{
		bool improveCacheLocality = true;
		ImGui::Checkbox("Improve Cache Locality", &improveCacheLocality);
		bool limitBoneWeights = true;
		ImGui::Checkbox("Limit Bone Weights", &limitBoneWeights);
		bool removeRedundantMaterials = true;
		ImGui::Checkbox("Remove Redundant Materials", &removeRedundantMaterials);
		bool splitLargeMeshes = true;
		ImGui::Checkbox("Split Large Meshes", &splitLargeMeshes);
		bool findDegenerates = true;
		ImGui::Checkbox("Find Degenerates", &findDegenerates);
		bool findInvalidData = true;
		ImGui::Checkbox("Find Invalid Data", &findInvalidData);

		if (currentConfiguration > 1)
		{
			bool findInstances = true;
			ImGui::Checkbox("Find Instances", &findInstances);
			bool validateDataStructure = true;
			ImGui::Checkbox("Validate Data Structure", &validateDataStructure);
			bool optimizeMeshes = true;
			ImGui::Checkbox("Optimize Meshes", &optimizeMeshes);
		}	
	}	

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
}

void PanelImportPreferences::TextureNode() const
{
	ImGui::Text("Import Options");
	ImGui::Separator();
	ImGui::Spacing();

	const char* compression[] = { "DXT1", "DXT2", "DXT3", "DXT4", "DXT5" };
	static int currentCompression = 0;
	ImGui::PushItemWidth(200.0f);
	ImGui::Combo("Compression", &currentCompression, compression, IM_ARRAYSIZE(compression));

	ImGui::Spacing();
	ImGui::Text("Load Options");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Wrap Mode");
	const char* wrap[] = { "Repeat", "Mirrored Repeat", "Clamp To Edge", "Clamp To Border" };
	static int currentWrapS = 0;
	static int currentWrapT = 0;
	ImGui::Combo("Wrap S", &currentWrapS, wrap, IM_ARRAYSIZE(wrap));
	ImGui::Combo("Wrap T", &currentWrapT, wrap, IM_ARRAYSIZE(wrap));

	ImGui::Text("Filter Mode");
	const char* filter[] = { "Nearest", "Linear", 
		"Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Nearest" };
	static int currentMinFilter = 0;
	static int currentMagFilter = 0;
	ImGui::Combo("Min Filter", &currentMinFilter, filter, IM_ARRAYSIZE(filter));
	ImGui::Combo("Mag Filter", &currentMagFilter, filter, IM_ARRAYSIZE(filter));
	ImGui::PopItemWidth();

	if (currentMinFilter > 1 || currentMagFilter > 1)
		ImGui::TextColored(YELLOW, "Mip Maps will be generated");

	if (App->materialImporter->IsAnisotropySupported())
	{
		float anisotropy = 0.0f;
		ImGui::SliderFloat("Anisotropy", &anisotropy, 0.0f, App->materialImporter->GetLargestSupportedAnisotropy());
	}
}

#endif // GAME