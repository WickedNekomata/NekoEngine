#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

enum PrimitiveTypes
{
	PrimitiveNoType,
	PrimitiveTypeRay,
	PrimitiveTypePlane,
	PrimitiveTypeCircle,
	PrimitiveTypeCube,
	PrimitiveTypeCylinder,
	PrimitiveTypeFrustum,
};

// Primitive: drawn using Vertex Array
class Primitive
{
public:

	Primitive();
	Primitive(math::float3 position, PrimitiveTypes type);
	virtual ~Primitive();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	virtual void	RenderAxis() const;
	void			EnableAxis(bool enable) { axis = enable; };
	PrimitiveTypes	GetType() const;

protected:

	math::float3 position = { 0.0f, 0.0f, 0.0f };
	PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType;

	math::float3 size = { 0.0f, 0.0f, 0.0f };

	GLfloat* vertices = nullptr;
	GLuint verticesID = 0;

	bool axis = false;
};

// PrimitiveIndex: drawn using Vertex Array with indices
class PrimitiveIndex : public Primitive
{
public:

	PrimitiveIndex();
	PrimitiveIndex(math::float3 position, PrimitiveTypes type);
	~PrimitiveIndex();

	virtual void InnerRender() const;

protected:

	GLubyte* indices = nullptr;
	GLuint indicesID = 0;
	GLsizei indicesSize = 0;
};

// Primitive --------------------------------------------------

// Ray
class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(math::float3 startPos = math::float3(0.0f, 0.0f, 0.0f), math::float3 endPos = math::float3(1.0f, 1.0f, 1.0f));
	void InnerRender() const;
};

// PrimitiveIndex --------------------------------------------------

// Circle
class PrimitiveCircle : public PrimitiveIndex
{
public:

	PrimitiveCircle(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float radius = 1.0f, uint sides = 8);

private:

	float radius = 0.0f;
	uint sides = 0;
};

// Plane
class PrimitivePlane : public PrimitiveIndex
{
public:

	PrimitivePlane(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float sizeX = 100.0f, float sizeZ = 100.0f);
};

// Cube
class PrimitiveCube : public PrimitiveIndex
{
public:

	PrimitiveCube(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), math::float3 size = math::float3(1.0f, 1.0f, 1.0f));
};

/*
class PrimitiveCylinder : public PrimitiveIndex
{
	PrimitiveCylinder(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float height = 1.0f, float radius = 1.0f, uint sides = 5);
	~PrimitiveCylinder();
	void InnerRender() const;
};
*/

// Frustum
class PrimitiveFrustum : public PrimitiveIndex
{
public:

	PrimitiveFrustum(math::float3 startPosition = math::float3(0.0f, 0.0f, 0.0f), float startSizeX = 1.f, float startSizeY = 1.f, math::float3 endPosition = math::float3(0.0f, 0.0f, 0.0f), float endSizeX = 1.f, float endSizeY = 1.f);
};