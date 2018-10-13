#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

enum PrimitiveTypes
{
	PrimitiveNoType,
	PrimitiveTypeRay,
	PrimitiveTypeAxis,
	PrimitiveTypePlane,
	PrimitiveTypeGrid,
	PrimitiveTypeCircle,
	PrimitiveTypeCube,
	PrimitiveTypeSphere,
	PrimitiveTypeCylinder,
	PrimitiveTypeCone,
	PrimitiveTypeArrow,
	PrimitiveTypeFrustum,
};

struct Transform 
{
	// Position
	math::float3 startPosition = { 0.0f, 0.0f, 0.0f };
	math::float3 position = { 0.0f, 0.0f, 0.0f };

	// Rotation
	float angle = 0.0f;
	math::float3 u = { 0.0f, 0.0f, 0.0f };

	// TODO: Scale
};

struct Color;
class PrimitiveAxis;

class Primitive
{
public:

	Primitive(PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType);
	virtual ~Primitive();

	virtual void Render() const;

	// -----

	PrimitiveTypes GetType() const;

	virtual void SetPosition(const math::float3& position);
	virtual void SetRotation(float angle, const math::float3& u); // angle in degrees
	virtual void SetColor(const Color& color);

	void SetWireframeMode(bool wireframeMode);
	void ShowAxis(bool showAxis);

protected:

	virtual void InnerRender() const;

protected:

	PrimitiveTypes type = PrimitiveTypes::PrimitiveNoType;
	Transform transform;
	Color color = White;

	bool wireframeMode = false;
	bool showAxis = false;
	PrimitiveAxis* axis = nullptr;

	// Draw (Vertex Array with indices)
	float* vertices = nullptr;
	uint verticesID = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;
};

// Ray --------------------------------------------------
class PrimitiveRay : public Primitive
{
public:

	PrimitiveRay(const math::float3& direction = math::float3(0.0f, 0.0f, -1.0f), float length = 100.0f, const math::float3& position = math::float3(0.0f, 0.0f, 0.0f));

private:
	
	void InnerRender() const;

private:

	math::float3 direction = { 0.0f, 0.0f, 0.0f };
	float length = 0.0f;
};

// Axis --------------------------------------------------
class PrimitiveAxis : public Primitive
{
public:

	PrimitiveAxis(const math::float3& position = math::float3(0.0f, 0.0f, 0.0f));
	~PrimitiveAxis();

private:

	void InnerRender() const;

private:

	PrimitiveRay* x = nullptr;
	PrimitiveRay* y = nullptr;
	PrimitiveRay* z = nullptr;
};

// Circle --------------------------------------------------
class PrimitiveCircle : public Primitive
{
public:

	PrimitiveCircle(float radius = 1.0f, uint sides = 8);

private:

	float radius = 0.0f;
	uint sides = 0;
};

// Plane --------------------------------------------------
class PrimitivePlane : public Primitive
{
public:

	PrimitivePlane(const math::float2& size = math::float2(100.0f, 100.0f));

private:

	math::float2 size; // x and z size
};

// Grid --------------------------------------------------
class PrimitiveGrid : public Primitive
{
public:

	PrimitiveGrid(uint quadSize = 1, uint quadsX = 100, uint quadsZ = 100);

private:

	void InnerRender() const;

private:

	uint quadSize = 0;
	uint quadsX = 0;
	uint quadsZ = 0;
};

// Cube --------------------------------------------------
class PrimitiveCube : public Primitive
{
public:

	PrimitiveCube(const math::float3& size = math::float3(1.0f, 1.0f, 1.0f), const math::float3& position = math::float3(0.0f, 0.0f, 0.0f));

private:

	math::float3 size = { 0.0f, 0.0f, 0.0f };
};

// Sphere --------------------------------------------------
class PrimitiveSphere : public Primitive
{
public:

	PrimitiveSphere(float radius = 1.0f, uint segments = 8);

private:

	float radius = 0.0f;
	uint segments = 0;
};

// Cylinder --------------------------------------------------
class PrimitiveCylinder : public Primitive
{
public:

	PrimitiveCylinder(float height = 5.0f, float radius = 1.0f, uint sides = 8);

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;
};

// Cone --------------------------------------------------
class PrimitiveCone : public Primitive
{
public:

	PrimitiveCone(float height = 5.0f, float radius = 1.0f, uint sides = 5, const math::float3& position = math::float3(0.0f, 0.0f, 0.0f));

private:

	float height = 0.0f;
	float radius = 0.0f;
	uint sides = 0;
};

// Arrow --------------------------------------------------
class PrimitiveArrow : public Primitive
{
public:

	PrimitiveArrow(float lineLength = 5.0f, float coneHeight = 1.0f, float coneRadius = 1.0f, uint coneSides = 8);
	~PrimitiveArrow();

private:

	void InnerRender() const;

private:

	PrimitiveCone* cone = nullptr;
	PrimitiveRay* line = nullptr;
};

// Frustum --------------------------------------------------
class PrimitiveFrustum : public Primitive
{
public:

	PrimitiveFrustum(const math::float2& startSize = math::float2(0.5f, 0.5f), const math::float3& endPosition = math::float3(0.0f, 0.0f, -1.0f), const math::float2& endSize = math::float2(1.0f, 1.0f));

private:

	math::float2 startSize = { 0.0f, 0.0f };
	math::float2 endSize = { 0.0f, 0.0f };

	math::float3 endPosition = { 0.0f, 0.0f, 0.0f };
};

// TODO: Capsule