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
	ImGui::Text("Agent Radius");
	ImGui::SameLine();
	ImGui::DragFloat("##ARadius", &radius, 0.1f);

	static float height = 1.0f;
	ImGui::Text("Agent Height");
	ImGui::SameLine();
	ImGui::DragFloat("##AHeight", &height, 0.1f);

	static float maxAcceleration = 8.0f;
	ImGui::Text("Max Acceleration");
	ImGui::SameLine();
	ImGui::DragFloat("##MAcc", &maxAcceleration, 0.1f);

	static float maxSpeed = 3.5f;
	ImGui::Text("Max Speed");
	ImGui::SameLine();
	ImGui::DragFloat("##MSpeed", &maxSpeed, 0.1f);

	static unsigned int params = 0;

	ImGui::Text("Anticipate Turns");
	ImGui::SameLine();
	ImGui::CheckboxFlags("##ATurns", &params, 1);

	ImGui::Text("Obstacle Avoidance");
	ImGui::SameLine();
	ImGui::CheckboxFlags("##OAvoidance", &params, 2);

	ImGui::Text("Separation");
	ImGui::SameLine();
	ImGui::CheckboxFlags("##Separation", &params, 4);
}

void ComponentNavAgent::OnInternalSave(JSON_Object* file)
{
}

void ComponentNavAgent::OnLoad(JSON_Object* file)
{
}
