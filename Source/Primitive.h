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

	virtual void	Render() const;
	virtual void	InnerRender() const;
	virtual void	RenderAxis() const;
	void			EnableAxis(bool enable) { axis = enable; };
	PrimitiveTypes	GetType() const;

protected:

	math::float3 position = { 0,0,0 };
	math::float3 size = { 0,0,0 };
	PrimitiveTypes type;
	GLuint verticesID = 0;
	GLfloat* vertices = nullptr;

	bool axis = false;
};

class PrimitiveCube : public Primitive
{
public:

	PrimitiveCube(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), math::float3 size = math::float3(1.0f, 1.0f, 1.0f));
	~PrimitiveCube();
	void InnerRender() const;

	math::float3 GetSize() const;

private:

	GLuint indicesID = 0;
	GLsizei indicesSize = 0;
};

class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(math::float3 startPos = math::float3(0.0f, 0.0f, 0.0f), math::float3 endPos = math::float3(1.0f, 1.0f, 1.0f));
	~PrimitiveRay();
	void InnerRender() const;
};

class PrimitivePlane : public Primitive
{
public:

	PrimitivePlane(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float sizeX = 100.0f, float sizeZ = 100.0f);
	~PrimitivePlane();
	void InnerRender() const;

private:
	GLuint indicesID = 0;
	GLsizei indicesSize = 0;
};