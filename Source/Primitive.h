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
	PrimitiveTypeSphere,
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

	virtual void Render() const;

	virtual math::float3 GetSize() const = 0;

	void EnableAxis(bool enable) { axis = enable; };
	PrimitiveTypes GetType() const;

protected:

	virtual void InnerRender() const;
	virtual void RenderAxis() const;

protected:

	math::float3 position = { 0.0f, 0.0f, 0.0f };
	PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType;

	bool axis = false;

	// Draw (Vertex Array)
	GLfloat* vertices = nullptr;
	GLuint verticesID = 0;
};

// PrimitiveIndex: drawn using Vertex Array with indices
class PrimitiveIndex : public Primitive
{
public:

	PrimitiveIndex();
	PrimitiveIndex(math::float3 position, PrimitiveTypes type);
	~PrimitiveIndex();

	virtual math::float3 GetSize() const = 0;

protected:

	virtual void InnerRender() const;

protected:

	// Draw (Vertex Array with indices)
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

	math::float3 GetSize() const { return math::float3(0.0f, 0.0f, 0.0f); }

private:

	void InnerRender() const;

private:

	math::float3 endPos = { 0.0f,0.0f,0.0f };
};

// PrimitiveIndex --------------------------------------------------

// Circle
class PrimitiveCircle : public PrimitiveIndex
{
public:

	PrimitiveCircle(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float radius = 1.0f, uint sides = 8);

	math::float3 GetSize() const { return math::float3(radius * 2.0f, radius * 2.0f, 1.0f); }

private:

	float radius = 0.0f;
	uint sides = 0;
};

// Plane
class PrimitivePlane : public PrimitiveIndex
{
public:

	PrimitivePlane(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float sizeX = 100.0f, float sizeZ = 100.0f);

	math::float3 GetSize() const { return math::float3(sizeX, 1.0f, sizeZ); }

private:

	float sizeX = 0.0f;
	float sizeZ = 0.0f;
};

// Cube
class PrimitiveCube : public PrimitiveIndex
{
public:

	PrimitiveCube(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), math::float3 size = math::float3(1.0f, 1.0f, 1.0f));

	math::float3 GetSize() const { return size; }

private:

	math::float3 size = { 0.0f,0.0f,0.0f };
};

// Sphere
class PrimitiveSphere : public PrimitiveIndex
{
public:

	PrimitiveSphere(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float radius = 1.0f);
	~PrimitiveSphere();

	math::float3 GetSize() const { return math::float3(radius * 2.0f, radius * 2.0f, radius * 2.0f); }

private:

	void InnerRender() const;

private:

	float radius = 0.0f;

	PrimitiveCircle* verticalCircle = nullptr;
	PrimitiveCircle* horizontalCircle = nullptr;
};

// Cylinder
class PrimitiveCylinder : public PrimitiveIndex
{
public:

	PrimitiveCylinder(math::float3 position = math::float3(0.0f, 0.0f, 0.0f), float height = 1.0f, float radius = 1.0f, uint sides = 8);
	~PrimitiveCylinder();

	math::float3 GetSize() const { return math::float3(radius * 2.0f, height, radius * 2.0f); }

private:

	void InnerRender() const;

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;

	PrimitiveCircle* topCap = nullptr;
	PrimitiveCircle* bottomCap = nullptr;
};

// Frustum
class PrimitiveFrustum : public PrimitiveIndex
{
public:

	PrimitiveFrustum(math::float3 startPosition = math::float3(0.0f, 0.0f, 0.0f), float startSizeX = 1.f, float startSizeY = 1.f, math::float3 endPosition = math::float3(0.0f, 0.0f, 0.0f), float endSizeX = 1.f, float endSizeY = 1.f);

	math::float3 GetSize() const { return math::float3(startSizeX, startSizeY, 1.0f); }

private:

	float startSizeX = 0.0f;
	float startSizeY = 0.0f;

	float endSizeX = 0.0f;
	float endSizeY = 0.0f;

	math::float3 endPosition = { 0.0f,0.0f,0.0f };
};