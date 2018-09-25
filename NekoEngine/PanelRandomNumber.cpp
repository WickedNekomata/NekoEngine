#include "PanelRandomNumber.h"

#include "ImGui/imgui.h"

#include <math.h>

PanelRandomNumber::PanelRandomNumber(char* name) : Panel(name)
{
}

PanelRandomNumber::~PanelRandomNumber()
{
}

bool PanelRandomNumber::Draw()
{
	ImGui::SetNextWindowSize({ 200,200 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Random Number Gen", false, inspectorFlags);

	if (ImGui::TreeNode("Random Int"))
	{
		ImGui::Text("Min: -50");
		static int minValue = -50;
		ImGui::Text("Max: 100");
		static int maxValue = 100;

		if (ImGui::Button("Generate"))
		{
			uint32_t bound;
			bound = maxValue - minValue + 1;
			rng = pcg32_boundedrand_r(&rngBound, bound);
			rng -= abs(minValue);
		}
		ImGui::Text("%d", rng);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Random Float"))
	{
		static float rngFloat = 0;
		if (ImGui::Button("Generate"))
		{
			rngFloat = ldexp(pcg32_random_r(&rngSeedFloat), -32);
		}
		ImGui::Text("%f", rngFloat);
		ImGui::TreePop();
	}

	ImGui::End();

	return true;
}
