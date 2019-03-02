#include "ComponentLight.h"


#include "imgui/imgui.h"

ComponentLight::ComponentLight(GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
}

ComponentLight::ComponentLight(const ComponentLight& componentLight, GameObject* parent) : Component(parent, ComponentTypes::LightComponent)
{
	lightType = componentLight.lightType;
	intensity = componentLight.intensity;
}

ComponentLight::~ComponentLight()
{
}

void ComponentLight::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Type");
	ImGui::SameLine();
	ImGui::PushItemWidth(100.0f);
	const char* lights[] = { "Directional", "Point", "Spot" };
	ImGui::Combo("##Light Type", (int*)&lightType, lights, IM_ARRAYSIZE(lights));
	ImGui::PopItemWidth();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Intensity");
	ImGui::SameLine();
	ImGui::DragScalar("##Light Intesity", ImGuiDataType_::ImGuiDataType_U32, &intensity, 1.0f);
#endif
}

uint ComponentLight::GetInternalSerializationBytes()
{
	return sizeof(int) * 2;
}

void ComponentLight::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(int);
	memcpy(cursor, &lightType, bytes);
	memcpy(cursor, &intensity, bytes);
}

void ComponentLight::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(int);
	memcpy(&lightType, cursor, bytes);
	memcpy(&intensity, cursor, bytes);
}
