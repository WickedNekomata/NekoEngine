#include "ComponentNavAgent.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleNavigation.h"

#ifndef GAMEMODE
#include "imgui/imgui.h"
#endif

ComponentNavAgent::ComponentNavAgent(GameObject* parent) : Component(parent, ComponentTypes::NavAgentComponent)
{
	App->navigation->AddComponent(this);
}

ComponentNavAgent::ComponentNavAgent(const ComponentNavAgent& componentNavAgent) :
	Component(componentNavAgent.parent, ComponentTypes::NavAgentComponent)
{
	memcpy(&(ComponentNavAgent&)componentNavAgent, this, sizeof(ComponentNavAgent));
	AddAgent();
	App->navigation->AddComponent(this);
}

ComponentNavAgent::~ComponentNavAgent()
{
	if (App->navigation->IsCrowInitialized())
		App->navigation->RemoveAgent(index);

	App->navigation->EraseComponent(this);
}

void ComponentNavAgent::Update()
{
}

void ComponentNavAgent::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("NavAgent", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Radius");
		ImGui::SameLine();
		ImGui::DragFloat("##ARadius", &radius, 0.1f);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Agent Height");
		ImGui::SameLine();
		ImGui::DragFloat("##AHeight", &height, 0.1f);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max Acceleration");
		ImGui::SameLine();
		ImGui::DragFloat("##MAcc", &maxAcceleration, 0.1f);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max Speed");
		ImGui::SameLine();
		ImGui::DragFloat("##MSpeed", &maxSpeed, 0.1f);

		ImGui::CheckboxFlags("Anticipate Turns", &params, 1);
		ImGui::CheckboxFlags("Optimize Visibility", &params, 8);
		ImGui::CheckboxFlags("Optimize Topology", &params, 16);
		ImGui::CheckboxFlags("Obstacle Avoidance", &params, 2);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Avoidance Quality");
		ImGui::SameLine();
		ImGui::SliderInt("##AQuality", &avoidanceQuality, 0.0f, 3.0f);

		ImGui::CheckboxFlags("Separation", &params, 4);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Separation Weight");
		ImGui::SameLine();
		ImGui::DragFloat("##SWeight", &separationWeight, 0.1f, 0.0f, 30.0f);
	}
#endif
}

void ComponentNavAgent::AddAgent()
{
	math::float3 pos;
	math::float3x3 rotation;
	math::float3 scale;
	GetParent()->transform->GetGlobalMatrix().Decompose(pos, rotation, scale);
	float collisionQueryRange = radius * 12.0f;
	float pathOptimizationRange = radius * 30.0f;
	index = App->navigation->AddAgent(pos.ptr(), radius, height, maxAcceleration, maxSpeed,
									  collisionQueryRange, pathOptimizationRange, params,
									  avoidanceQuality);
}

bool ComponentNavAgent::UpdateParams() const
{
	float collisionQueryRange = radius * 12.0f;
	float pathOptimizationRange = radius * 30.0f;
	return App->navigation->UpdateAgentParams(index, radius, height, maxAcceleration, maxSpeed,
		   collisionQueryRange, pathOptimizationRange, params,
		   avoidanceQuality);;
}

void ComponentNavAgent::SetDestination(const float* pos) const
{
	App->navigation->SetDestination(pos, index);
}

bool ComponentNavAgent::IsWalking()
{
	return App->navigation->IsWalking(index);
}

uint ComponentNavAgent::GetInternalSerializationBytes()
{
	return sizeof(float) * 5 + sizeof(unsigned int) + sizeof(int) * 2;
}

void ComponentNavAgent::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(float);
	memcpy(cursor, &radius, bytes);
	cursor += bytes;

	memcpy(cursor, &height, bytes);
	cursor += bytes;

	memcpy(cursor, &maxAcceleration, bytes);
	cursor += bytes;

	memcpy(cursor, &maxSpeed, bytes);
	cursor += bytes;

	memcpy(cursor, &separationWeight, bytes);
	cursor += bytes;

	bytes = sizeof(unsigned int);
	memcpy(cursor, &params, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(cursor, &avoidanceQuality, bytes);
	cursor += bytes;

	memcpy(cursor, &index, bytes);
	cursor += bytes;
}

void ComponentNavAgent::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(float);
	memcpy(&radius, cursor, bytes);
	cursor += bytes;

	memcpy(&height, cursor, bytes);
	cursor += bytes;

	memcpy(&maxAcceleration, cursor, bytes);
	cursor += bytes;

	memcpy(&maxSpeed, cursor, bytes);
	cursor += bytes;

	memcpy(&separationWeight, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(unsigned int);
	memcpy(&params, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	memcpy(&avoidanceQuality, cursor, bytes);
	cursor += bytes;

	memcpy(&index, cursor, bytes);
	cursor += bytes;
}
