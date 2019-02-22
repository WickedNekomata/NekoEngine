#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Math\Quat.h"
#include "MathGeoLib\include\Math\float4x4.h"

#include "glew\include\GL\glew.h"

#include "Uniforms.h"

#include <vector>

#define MAX_TEXTURES 3 // Albedo (Ambient + Diffuse), Specular, normal

class ResourceShaderProgram;

struct MaterialResource1
{
	uint res = 0;
	math::float4x4 matrix = math::float4x4::identity;
	bool checkers = false;

	uint id = 0;
	uint width = 0;
	uint height = 0;

	bool operator==(const MaterialResource1 rhs)
	{
		return this->res == rhs.res && matrix.Equals(rhs.matrix);
	}
};

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent);
	ComponentMaterial(const ComponentMaterial& componentMaterial);
	~ComponentMaterial();

	void Update();

	void SetResource(uint res_uuid, uint position);
	void UpdateUniforms();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

private:

	void EditCurrentResMatrixByIndex(int i);

public:

	GLuint shaderProgramUUID = 0;
	std::vector<Uniform> uniforms;
	std::vector<MaterialResource1> res;
	float color[4] = { 1.0f,1.0f,1.0f,255.0f };
};

#endif