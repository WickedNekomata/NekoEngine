#include "ComponentMaterial.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"
#include "GameObject.h"
#include "Panel.h"
#include "Resource.h"
#include "ResourceTexture.h"
#include "ResourceShaderProgram.h"

#include "imgui\imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentType::MaterialComponent) 
{
	res.reserve(App->renderer3D->GetMaxTextureUnits());

	// Default texture (0)
	MaterialResource texture;
	res.push_back(texture);
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentType::MaterialComponent)
{
	res = componentMaterial.res;

	for (uint i = 0; i < res.size(); ++i)
	{
		res[i].res = 0;
		SetResource(componentMaterial.res[i].res, i);
	}

	for (uint i = 0; i < 4; ++i)
		color[i] = componentMaterial.color[i];
}

ComponentMaterial::~ComponentMaterial()
{
	parent->materialRenderer = nullptr;

	for (uint i = 0; i < res.size(); ++i)
		SetResource(0, i);
}

void ComponentMaterial::Update() {}

void ComponentMaterial::SetResource(uint res_uuid, uint position)
{
	if (res[position].res != 0)
		App->res->SetAsUnused(res[position].res);

	if (res_uuid != 0) {
		if (App->res->SetAsUsed(res_uuid) == -1)
			return;
	}

	res[position].res = res_uuid;
}

void ComponentMaterial::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Material Renderer");
	ImGui::Spacing();

	// Shader Program
	ImGui::Text("Shader");
	ImGui::SameLine();

	ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUUID);

	ImGui::PushID("shader");
	ImGui::Button(shaderProgram != nullptr ? shaderProgram->GetName() : "Default Shader", ImVec2(150.0f, 0.0f));
	ImGui::PopID();

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%u", shaderProgram != nullptr ? shaderProgram->shaderProgram : App->shaderImporter->GetDefaultShaderProgram());
		ImGui::EndTooltip();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_PROGRAM"))
		{
			ResourceShaderProgram* payload_n = *(ResourceShaderProgram**)payload->Data;
			payload_n->GetUniforms(uniforms);
			shaderProgramUUID = payload_n->GetUUID();
		}
		ImGui::EndDragDropTarget();
	}

	for (int i = 0, n = uniforms.size(); i < n; ++i)
	{
		Uniform* uniform = uniforms[i];
		ImGui::Text(uniform->name);
		ImGui::SameLine();

		char itemName[] = { "##uniformX" };
		itemName[10] = i;

		switch (uniform->type)
		{
		case Uniforms_Values::FloatU_value:
			ImGui::InputFloat(itemName, &uniform->floatU.value);
			break;
		case Uniforms_Values::IntU_value:
			ImGui::InputInt(itemName, (int*)&uniform->intU.value);
			break;
		case Uniforms_Values::Vec2FU_value:
		{
			float v[] = { uniform->vec2FU.value.x, uniform->vec2FU.value.y };
			ImGui::InputFloat2(itemName, v);
			break;
		}
		case Uniforms_Values::Vec3FU_value:
		{
			float v[] = { uniform->vec3FU.value.x, uniform->vec3FU.value.y , uniform->vec3FU.value.z };
			ImGui::InputFloat3(itemName, v);
			break;
		}
		case Uniforms_Values::Vec4FU_value:
		{
			float v[] = { uniform->vec4FU.value.x, uniform->vec4FU.value.y , uniform->vec4FU.value.z, uniform->vec4FU.value.w };
			ImGui::InputFloat4(itemName, v);
			break;
		}
		case Uniforms_Values::Vec2IU_value:
		{
			int v[] = { uniform->vec2IU.value.x, uniform->vec2IU.value.y };
			ImGui::InputInt2(itemName, v);
			break;
		}
		case Uniforms_Values::Vec3IU_value:
		{
			int v[] = { uniform->vec3IU.value.x, uniform->vec3IU.value.y , uniform->vec3IU.value.z };
			ImGui::InputInt3(itemName, v);
			break;
		}
		case Uniforms_Values::Vec4IU_value:
		{
			int v[] = { uniform->vec4IU.value.x, uniform->vec4IU.value.y , uniform->vec4IU.value.z, uniform->vec4IU.value.w };
			ImGui::InputInt4(itemName, v);
			break;
		}
		}
	}

	if (ImGui::Button("USE DEFAULT SHADER"))
		shaderProgramUUID = 0;

	// Textures
	ImGui::Spacing();

	ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_NoInputs);

	for (uint i = 0; i < res.size(); ++i)
	{
		ImGui::Text("Texture %i", i + 1);
		ImGui::SameLine();

		uint id = 0;
		uint width = 0;
		uint height = 0;

		char itemName[DEFAULT_BUF_SIZE];

		const ResourceTexture* resource = (const ResourceTexture*)App->res->GetResource(res[i].res);
		if (resource != nullptr)
		{
			id = resource->id;
			width = resource->width;
			height = resource->height;

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##%i", resource->GetName(), i);
		}
		else if (res[i].checkers)
		{
			id = App->materialImporter->GetCheckers();
			width = CHECKERS_WIDTH;
			height = CHECKERS_HEIGHT;

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Checkers##%i", i);
		}
		else
		{
			id = App->materialImporter->GetDefaultTexture();
			width = REPLACE_ME_WIDTH;
			height = REPLACE_ME_HEIGHT;

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Replace Me!##%i", i);
		}

		ImGui::Button(itemName, ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", id);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetResource(payload_n, i);
				res[i].checkers = false;
			}
			ImGui::EndDragDropTarget();
		}

		bool minusClicked = false;

		if (res.size() > 1)
		{
			ImGui::SameLine();

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "-##%i", i);
			if (ImGui::Button(itemName))
				minusClicked = true;
		}

		ImGui::Image((void*)(intptr_t)id, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "Matrix##%i", i);
		ImGui::Button(itemName);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Edit Texture Matrix");

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "Edit Matrix##%i", i);
		if (ImGui::BeginPopupContextItem(itemName, 0)) 
		{
			EditCurrentResMatrixByIndex(i);
			ImGui::EndPopup();
		}

		ImGui::TextColored(BLUE, "%u x %u", width, height);
		ImGui::Spacing();

		if (minusClicked)
		{
			SetResource(0, i);
			res.erase(res.begin() + i);
		}

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "USE DEFAULT TEXTURE##%i", i);
		if (ImGui::Button(itemName))
		{
			SetResource(0, i);
			res[i].checkers = false;
		}

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "USE CHECKERS TEXTURE##%i", i);
		if (ImGui::Button(itemName))
		{
			SetResource(0, i);
			res[i].checkers = true;
		}
	}

	if (res.size() < App->renderer3D->GetMaxTextureUnits())
	{
		if (ImGui::Button("+")) 
		{
			MaterialResource newRes;
			res.push_back(newRes);
		}
	}
#endif
}

void ComponentMaterial::OnInternalSave(JSON_Object* file)
{
	JSON_Value* arrayValue = json_value_init_array();
	JSON_Array* jsonArray = json_value_get_array(arrayValue);
	json_array_append_number(jsonArray, color[0]);
	json_array_append_number(jsonArray, color[1]);
	json_array_append_number(jsonArray, color[2]);
	json_array_append_number(jsonArray, color[3]);
	json_object_set_value(file, "VertexColor", arrayValue);

	json_array_append_value(jsonArray, arrayValue);

	arrayValue = json_value_init_array();
	jsonArray = json_value_get_array(arrayValue);

	for (int i = 0; i < res.size(); ++i)
	{
		JSON_Value* newValue = json_value_init_object();
		JSON_Object* resource = json_value_get_object(newValue);

		json_object_set_number(resource, "res", res[i].res);

		math::float3 pos; math::Quat rot; math::float3 scale;
		res[i].matrix.Decompose(pos, rot, scale);
		json_object_set_number(resource, "posX", pos.x);
		json_object_set_number(resource, "posY", pos.y);
		json_object_set_number(resource, "posZ", pos.z);

		json_object_set_number(resource, "rotX",rot.x);
		json_object_set_number(resource, "rotY",rot.y);
		json_object_set_number(resource, "rotZ",rot.z);
		json_object_set_number(resource, "rotW",rot.w);

		json_object_set_number(resource, "scaleX", scale.x);
		json_object_set_number(resource, "scaleY", scale.y);
		json_object_set_number(resource, "scaleZ", scale.z);

		json_array_append_value(jsonArray, newValue);
	}
	json_object_set_value(file, "TexturesResources", arrayValue);
}

void ComponentMaterial::OnLoad(JSON_Object* file)
{
	JSON_Array* jsonArray = json_object_get_array(file, "VertexColor");

	uint arraySize = json_array_get_count(jsonArray);

	for (int i = 0; i < arraySize; i++)
		color[i] = json_array_get_number(jsonArray, i);

	jsonArray = json_object_get_array(file, "TexturesResources");

	arraySize = json_array_get_count(jsonArray);

	while (res.size() < arraySize)
	{
		MaterialResource newRes;
		res.push_back(newRes);
	}

	for (int i = 0; i < arraySize; i++) {
		JSON_Object* rObject = json_array_get_object(jsonArray, i);

		uint newRes = json_object_get_number(rObject, "res");
		if (newRes == 0 || App->res->GetResource(newRes) != nullptr)
			SetResource(newRes, i);

		math::float3 pos; math::Quat rot; math::float3 scale;

		pos.x = json_object_get_number(rObject, "posX");
		pos.y = json_object_get_number(rObject, "posY");
		pos.z = json_object_get_number(rObject, "posZ");

		rot.x = json_object_get_number(rObject, "rotX");
		rot.y = json_object_get_number(rObject, "rotY");
		rot.z = json_object_get_number(rObject, "rotZ");
		rot.w = json_object_get_number(rObject, "rotW");

		scale.x = json_object_get_number(rObject, "scaleX");
		scale.y = json_object_get_number(rObject, "scaleY");
		scale.z = json_object_get_number(rObject, "scaleZ");

		res[i].matrix = math::float4x4::FromTRS(pos, rot, scale);
	}
}

void ComponentMaterial::EditCurrentResMatrixByIndex(int i)
{
#ifndef GAMEMODE
	char itemName[DEFAULT_BUF_SIZE];
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "Reset##%i", i);
	if (ImGui::Button(itemName))
		res[i].matrix = math::float4x4::identity;

	math::float3 pos = math::float3::zero;
	math::Quat rot = math::Quat::identity;
	math::float3 scale = math::float3::one;
	res[i].matrix.Decompose(pos, rot, scale);

	////////////////////////////
	const double f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;
	ImGui::Text("Position");

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##PosX%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&pos.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##PosY%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&pos.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##PosZ%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&pos.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	ImGui::Text("Rotation");
	math::float3 axis;
	float angle;
	rot.ToAxisAngle(axis, angle);
	axis *= angle;
	axis *= RADTODEG;
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##AxisAngleX%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&axis.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##AxisAngleY%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&axis.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##AxisAngleZ%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&axis.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);
	axis *= DEGTORAD;
	rot.SetFromAxisAngle(axis.Normalized(), axis.Length());

	ImGui::Text("Scale");

	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##ScaleX%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&scale.x, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##ScaleY%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&scale.y, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f); ImGui::SameLine();
	sprintf_s(itemName, DEFAULT_BUF_SIZE, "##ScaleZ%i", i);
	ImGui::PushItemWidth(50);
	ImGui::DragScalar(itemName, ImGuiDataType_Float, (void*)&scale.z, 0.1f, &f64_lo_a, &f64_hi_a, "%f", 1.0f);

	res[i].matrix = math::float4x4::FromTRS(pos, rot, scale);
#endif
}
