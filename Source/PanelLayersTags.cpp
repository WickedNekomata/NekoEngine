#include "PanelLayersTags.h"

#include "imgui/imgui.h"

PanelLayersTags::PanelLayersTags(char* name) : Panel(name)
{
}

PanelLayersTags::~PanelLayersTags()
{
}

bool PanelLayersTags::Draw()
{
	ImGui::Begin(name);

	if (ImGui::TreeNode("Tags"))
	{
		for (int it = 0; it < tags.size(); ++it)
			ImGui::Text("Tag%i			%s");

		ImGui::TreePop();
	}

	ImGui::End();

	return true;
}
