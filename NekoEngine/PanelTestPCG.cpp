#include "PanelTestPCG.h"

#include "ImGui/imgui.h"

#include <math.h>

PanelTestPCG::PanelTestPCG(char* name) : Panel(name) {}

PanelTestPCG::~PanelTestPCG() {}

bool PanelTestPCG::Draw()
{
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin(name, &enabled, inspectorFlags);

	if (ImGui::TreeNode("Bounded Int test"))
	{
		static int minValue = 0;
		ImGui::InputInt("Min Value", &minValue);
		static int maxValue = 100;
		ImGui::InputInt("Max Value", &maxValue);
		static int quantityInt = 100;
		ImGui::InputInt("Quantity", &quantityInt);

		if (ImGui::Button("Generate Histogram"))
		{
			randomIntNumbers.clear();
			uint32_t bound;
			bound = maxValue - minValue + 1;
			for (int i = 0; i < quantityInt; ++i)
			{
				rng = pcg32_boundedrand_r(&rngBound, bound);
				rng -= abs(minValue);
				randomIntNumbers.push_back(rng);
				plotBoundedHistogram = true;
			}
		}

		if (plotBoundedHistogram)
		{
			char title[20];
			sprintf_s(title, IM_ARRAYSIZE(title), "Plot Int:");
			ImGui::PlotHistogram("##BoundedInt", &randomIntNumbers.front(), randomIntNumbers.size(), 0, title, (float)minValue, (float)maxValue, ImVec2(310, 100));

		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Float test"))
	{
		static float rngFloat = 0.0f;
		static int quantityFloat = 100.0f;
	
		ImGui::InputInt("Quantity", &quantityFloat);

		if (ImGui::Button("Generate Histogram"))
		{
			randomFloatNumbers.clear();
			for (int i = 0; i < quantityFloat; ++i)
				randomFloatNumbers.push_back(ldexp(pcg32_random_r(&rngSeedFloat), -32));
			plotFloatHistogram = true;

		}

		if (plotFloatHistogram)
		{
			char title[20];
			sprintf_s(title, IM_ARRAYSIZE(title), "Plot floats");
			ImGui::PlotHistogram("##floats", &randomFloatNumbers.front(), randomFloatNumbers.size(), 0, title, 0.0f, 1.0f, ImVec2(310, 100));
		}
		ImGui::TreePop();
	}

	ImGui::End();

	return true;
}