#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

enum PrimitiveTypes
{
	PrimitiveNoType,
	PrimitiveTypeCube,
	PrimitiveTypeRay
};

class Primitive
{
public:

	Primitive();
	Primitive(math::float3 position);

	virtual void	Render() const;
	virtual void	InnerRender() const;
	PrimitiveTypes	GetType() const;

protected:

	PrimitiveTypes type;
	GLuint verticesID = 0;
	GLfloat* vertices = nullptr;
};

class PrimitiveCube : public Primitive
{
public:

	PrimitiveCube(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), math::float3 size = math::float3(1.0f, 1.0f, 1.0f));
	~PrimitiveCube();
	void InnerRender() const;

	math::float3 GetSize() const;

private:

	math::float3 size;

	GLuint indicesID = 0;
	GLsizei indicesSize = 0;
};

class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(math::float3 startPos = math::float3(0.0f, 0.0f, 0.0f), math::float3 endPos = math::float3(1.0f, 1.0f, 1.0f));
	~PrimitiveRay();
	void InnerRender() const;

private:
};