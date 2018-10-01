#pragma once

#include "Color.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

enum PrimitiveTypes
{
	PrimitiveTypePoint,
	PrimitiveTypeLine,

	PrimitiveTypeCube,
	PrimitiveTypeSphere,
	PrimitiveTypeCylinder
};

class Primitive
{
public:

	Primitive();
	Primitive(math::float3 position);

	virtual void	Render() const;
	virtual void	InnerRender() const;
	PrimitiveTypes	GetType() const;
	void			SetPosition(math::float3 position);
	void			SetRotation(float angle, const math::float3 &u);
	void			SetScale(math::float3 scale);

public:
	
	Color color;
	math::float4x4 transform;
	bool axis;

protected:

	PrimitiveTypes type;
};

/*
class PrimitivePoint : public Primitive
{
public:

	PrimitivePoint();
	PrimitivePoint(float x, float y, float z);
	void InnerRender() const;

public:

	math::float3 origin;
	math::float3 destination;
};

class PrimitiveLine : public Primitive
{
public:

	PrimitiveLine();
	PrimitiveLine(float x, float y, float z);
	void InnerRender() const;

public:

	math::float3 origin;
	math::float3 destination;
};
*/
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
	GLuint verticesID = 0;

	GLsizei indicesSize = 0;
	GLfloat* vertices;
};
/*
class PrimitiveSphere : public Primitive
{
public:

	PrimitiveSphere();
	PrimitiveSphere(float radius);
	void InnerRender() const;

public:

	float radius;
};

class PrimitiveCylinder : public Primitive
{
public:

	PrimitiveCylinder();
	PrimitiveCylinder(float radius, float height);
	void InnerRender() const;

public:

	float radius;
	float height;
};

class pPlane : public Primitive
{
public:

	pPlane();
	pPlane(float x, float y, float z, float d);
	void InnerRender() const;

public:

	math::float3 normal;
	float constant;
};
*/