#include "ComponentNavAgent.h"

#include "imgui/imgui.h"

ComponentNavAgent::ComponentNavAgent(GameObject* parent) : Component(parent, ComponentType::NavAgentComponent)
{
}

ComponentNavAgent::ComponentNavAgent(const ComponentNavAgent& componentTransform) :
	Component(componentTransform.parent, ComponentType::NavAgentComponent)
{
}

ComponentNavAgent::~ComponentNavAgent()
{
}

void ComponentNavAgent::Update()
{
}

void ComponentNavAgent::OnUniqueEditor()
{
	static float radius = 1.0f;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Radius");
	ImGui::SameLine();
	ImGui::DragFloat("##ARadius", &radius, 0.1f);

	static float height = 1.0f;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Agent Height");
	ImGui::SameLine();
	ImGui::DragFloat("##AHeight", &height, 0.1f);

	static float maxAcceleration = 8.0f;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max Acceleration");
	ImGui::SameLine();
	ImGui::DragFloat("##MAcc", &maxAcceleration, 0.1f);

	static float maxSpeed = 3.5f;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max Speed");
	ImGui::SameLine();
	ImGui::DragFloat("##MSpeed", &maxSpeed, 0.1f);

	static unsigned int params = 0;

	ImGui::CheckboxFlags("Anticipate Turns", &params, 1);

	ImGui::CheckboxFlags("Optimize Visibility", &params, 8);

	ImGui::CheckboxFlags("Optimize Topology", &params, 16);

	ImGui::CheckboxFlags("Obstacle Avoidance", &params, 2);

	ImGui::CheckboxFlags("Separation", &params, 4);

	static float separationWeight = 2.0f;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Separation Weight");
	ImGui::SameLine();
	ImGui::DragFloat("##SWeight", &separationWeight, 0.1f, 0.0f, 30.0f);

	static int avoidanceQuality = 3;
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Avoidance Quality");
	ImGui::SameLine();
	ImGui::SliderInt("##AQuality", &avoidanceQuality, 0.0f, 3.0f);

}

void ComponentNavAgent::AddAgent()
{
}

void ComponentNavAgent::SetDestination()
{
}

void ComponentNavAgent::OnInternalSave(JSON_Object* file)
{
}

void ComponentNavAgent::OnLoad(JSON_Object* file)
{
}
