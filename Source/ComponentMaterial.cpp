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

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent, ComponentTypes::MaterialComponent)
{
	if (MAX_TEXTURES <= App->renderer3D->GetMaxTextureUnits())
	{
		res.reserve(MAX_TEXTURES);

		for (uint i = 0; i < MAX_TEXTURES; ++i)
		{
			MaterialResource materialResource;
			res.push_back(materialResource);
		}
	}
}

ComponentMaterial::ComponentMaterial(const ComponentMaterial& componentMaterial) : Component(componentMaterial.parent, ComponentTypes::MaterialComponent)
{
	res = componentMaterial.res;

	for (uint i = 0; i < res.size(); ++i)
	{
		res[i].res = 0;
		SetResource(componentMaterial.res[i].res, i);
	}

	for (uint i = 0; i < 4; ++i)
		color[i] = componentMaterial.color[i];

	shaderProgramUUID = componentMaterial.shaderProgramUUID;

	for (uint i = 0; i < componentMaterial.uniforms.size(); ++i)
	{
		Uniform uniform;
		memcpy_s(&uniform, sizeof(Uniform), &componentMaterial.uniforms[i], sizeof(Uniform));
		uniforms.push_back(uniform);
	}
}

ComponentMaterial::~ComponentMaterial()
{
	parent->cmp_material = nullptr;

	for (uint i = 0; i < res.size(); ++i)
		SetResource(0, i);

	uniforms.clear();
}

void ComponentMaterial::Update() {}

void ComponentMaterial::SetResource(uint res_uuid, uint position)
{
	if (res[position].res != 0)
		App->res->SetAsUnused(res[position].res);

	if (res_uuid != 0) 
	{
		if (App->res->SetAsUsed(res_uuid) == -1)
			return;
	}

	res[position].res = res_uuid;
}

void ComponentMaterial::UpdateUniforms()
{
	uniforms.clear();
	const ResourceShaderProgram* program = (ResourceShaderProgram*)App->res->GetResource(shaderProgramUUID);
	if (program != nullptr)
		program->GetUniforms(uniforms);
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

	if (shaderProgram != nullptr && !shaderProgram->isValid)
	{
		shaderProgramUUID = 0;
		uniforms.clear();
	}

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
			if (payload_n->isValid)
			{
				uniforms.clear();
				payload_n->GetUniforms(uniforms);
				shaderProgramUUID = payload_n->GetUuid();
			}
			else
				DEPRECATED_LOG("Invalid shader program");
		}
		ImGui::EndDragDropTarget();
	}

	for (uint i = 0, n = uniforms.size(); i < n; ++i)
	{
		if (i == 0)
			ImGui::Text("Active Uniforms");

		Uniform uniform = uniforms[i];
		ImGui::Text(uniform.common.name);
		ImGui::SameLine();

		char itemName[] = { "##uniformX" };
		itemName[10] = i;
		ImGui::PushItemWidth(100.0f);
		switch (uniform.common.type)
		{
		case Uniforms_Values::FloatU_value:
			ImGui::InputFloat(itemName, &uniform.floatU.value);
			break;
		case Uniforms_Values::IntU_value:
			ImGui::InputInt(itemName, (int*)&uniform.intU.value);
			break;
		case Uniforms_Values::Vec2FU_value:
		{
			float v[] = { uniform.vec2FU.value.x, uniform.vec2FU.value.y };
			ImGui::InputFloat2(itemName, v);
			uniform.vec2FU.value.x = v[0];
			uniform.vec2FU.value.y = v[1];
			break;
		}
		case Uniforms_Values::Vec3FU_value:
		{
			float v[] = { uniform.vec3FU.value.x, uniform.vec3FU.value.y , uniform.vec3FU.value.z };
			ImGui::InputFloat3(itemName, v);
			uniform.vec3FU.value.x = v[0];
			uniform.vec3FU.value.y = v[1];
			uniform.vec3FU.value.z = v[2];
			break;
		}
		case Uniforms_Values::Vec4FU_value:
		{
			float v[] = { uniform.vec4FU.value.x, uniform.vec4FU.value.y , uniform.vec4FU.value.z, uniform.vec4FU.value.w };
			ImGui::InputFloat4(itemName, v);
			uniform.vec4FU.value.x = v[0];
			uniform.vec4FU.value.y = v[1];
			uniform.vec4FU.value.z = v[2];
			uniform.vec4FU.value.w = v[3];
			break;
		}
		case Uniforms_Values::Vec2IU_value:
		{
			int v[] = { uniform.vec2IU.value.x, uniform.vec2IU.value.y };
			ImGui::InputInt2(itemName, v);
			uniform.vec2IU.value.x = v[0];
			uniform.vec2IU.value.y = v[1];
			break;
		}
		case Uniforms_Values::Vec3IU_value:
		{
			int v[] = { uniform.vec3IU.value.x, uniform.vec3IU.value.y , uniform.vec3IU.value.z };
			ImGui::InputInt3(itemName, v);
			uniform.vec3IU.value.x = v[0];
			uniform.vec3IU.value.y = v[1];
			uniform.vec3IU.value.z = v[2]; 
			break;
		}
		case Uniforms_Values::Vec4IU_value:
		{
			int v[] = { uniform.vec4IU.value.x, uniform.vec4IU.value.y , uniform.vec4IU.value.z, uniform.vec4IU.value.w };
			ImGui::InputInt4(itemName, v);
			uniform.vec4IU.value.x = v[0];
			uniform.vec4IU.value.y = v[1];
			uniform.vec4IU.value.z = v[2];
			uniform.vec4IU.value.w = v[3];
			break;
		}
		}
		ImGui::PopItemWidth();
	}

	if (ImGui::Button("USE DEFAULT SHADER"))
	{
		shaderProgramUUID = 0;
		uniforms.clear();
	}

	// Textures
	ImGui::Spacing();

	ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_NoInputs);

	for (uint i = 0; i < res.size(); ++i)
	{
		switch (i)
		{
		case 0:
			ImGui::Text("Albedo");
			break;
		case 1:
			ImGui::Text("Specular");
			break;
		case 2:
			ImGui::Text("Normal Map");
			break;
		}
		ImGui::SameLine();

		char itemName[DEFAULT_BUF_SIZE];

		const ResourceTexture* resource = (const ResourceTexture*)App->res->GetResource(res[i].res);
		if (resource != nullptr)
		{
			res[i].id = resource->GetId();
			res[i].width = resource->GetWidth();
			res[i].height = resource->GetHeight();

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "%s##%i", resource->GetName(), i);
		}
		else if (res[i].checkers)
		{
			res[i].id = App->materialImporter->GetCheckers();
			res[i].width = CHECKERS_WIDTH;
			res[i].height = CHECKERS_HEIGHT;

			sprintf_s(itemName, DEFAULT_BUF_SIZE, "Checkers##%i", i);
		}
		else
		{
			if (i == 0)
			{
				res[i].id = App->materialImporter->GetDefaultTexture();
				res[i].width = REPLACE_ME_WIDTH;
				res[i].height = REPLACE_ME_HEIGHT;

				sprintf_s(itemName, DEFAULT_BUF_SIZE, "Replace Me!##%i", i);
			}
			else
				sprintf_s(itemName, DEFAULT_BUF_SIZE, "No Texture##%i", i);
		}

		ImGui::Button(itemName, ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res[i].id);
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

		ImGui::Image((void*)(intptr_t)res[i].id, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
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

		ImGui::TextColored(BLUE, "%u x %u", res[i].width, res[i].height);
		ImGui::Spacing();

		sprintf_s(itemName, DEFAULT_BUF_SIZE, "USE DEFAULT TEXTURE##%i", i);
		if (ImGui::Button(itemName))
		{
			SetResource(0, i);
			res[i].checkers = false;
		}

		if (i == 0)
		{
			sprintf_s(itemName, DEFAULT_BUF_SIZE, "USE CHECKERS TEXTURE##%i", i);
			if (ImGui::Button(itemName))
			{
				SetResource(0, i);
				res[i].checkers = true;
			}
		}
	}
#endif
}

uint ComponentMaterial::GetInternalSerializationBytes()
{
	size_t size = sizeof(GLuint);
	size += sizeof(uint);
	size += sizeof(Uniform) * uniforms.size();

	size += sizeof(uint);
	size += sizeof(MaterialResource) * res.size();

	size += sizeof(float) * 4;

	return size;
}

void ComponentMaterial::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(GLuint);
	memcpy(cursor, &shaderProgramUUID, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int numUniforms = uniforms.size();
	memcpy(cursor, &numUniforms, bytes);
	cursor += bytes;
	
	bytes = sizeof(Uniform) * uniforms.size();
	memcpy(cursor, uniforms.data(), bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int numRes = res.size();
	memcpy(cursor, &numRes, bytes);
	cursor += bytes;

	bytes = sizeof(MaterialResource) * res.size();
	memcpy(cursor, res.data(), bytes);
	cursor += bytes;

	bytes = sizeof(float) * 4;
	memcpy(cursor, color, bytes);
	cursor += bytes;
}

void ComponentMaterial::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(GLuint);
	memcpy(&shaderProgramUUID, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int numUniforms;
	memcpy(&numUniforms, cursor, bytes);
	cursor += bytes;

	uniforms.resize(numUniforms);

	bytes = sizeof(Uniform) * numUniforms;
	memcpy(uniforms.data(), cursor, bytes);
	cursor += bytes;

	bytes = sizeof(int);
	int numRes;
	memcpy(&numRes, cursor, bytes);
	cursor += bytes;

	res.resize(numRes);

	bytes = sizeof(MaterialResource) * numRes;
	memcpy(res.data(), cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float) * 4;
	memcpy(color, cursor, bytes);
	cursor += bytes;
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
