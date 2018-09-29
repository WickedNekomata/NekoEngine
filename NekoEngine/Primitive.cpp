#include "Globals.h"
#include "glew/include/GL/glew.h"
#include "Primitive.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

Primitive::Primitive() : transform(math::float4x4::identity), color(White), wire(false), axis(false), type(PrimitiveTypes::Primitive_Point)
{}

void Primitive::Render() const
{
	glPushMatrix();
	glMultMatrixf((GLfloat*)transform.Transposed().ptr());

	if (axis)
	{
		// Draw Axis Grid
		glLineWidth(2.0f);

		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
		glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
		glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
		glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

		glEnd();

		glLineWidth(1.0f);
	}

	glColor3f(color.r, color.g, color.b);

	if (wire)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	InnerRender();

	glPopMatrix();
}

void Primitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}

PrimitiveTypes Primitive::GetType() const
{
	return type;
}

void Primitive::SetPos(float x, float y, float z)
{
	transform = math::float4x4::Translate(x, y, z).ToFloat4x4() * transform;
}

void Primitive::SetRotation(float angle, const math::float3 &u)
{
	transform = math::float4x4::RotateAxisAngle(u, angle) * transform;
}

void Primitive::Scale(float x, float y, float z)
{
	transform = math::float4x4::Scale(x, y, z).ToFloat4x4() * transform;
}

// CUBE ============================================
pCube::pCube() : Primitive(), size(1.0f, 1.0f, 1.0f)
{
	type = PrimitiveTypes::Primitive_Cube;
}

pCube::pCube(float sizeX, float sizeY, float sizeZ) : Primitive(), size(sizeX, sizeY, sizeZ)
{
	type = PrimitiveTypes::Primitive_Cube;
}

void pCube::InnerRender() const
{	
	float sx = size.x * 0.5f;
	float sy = size.y * 0.5f;
	float sz = size.z * 0.5f;

	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-sx, -sy, sz);
	glVertex3f( sx, -sy, sz);
	glVertex3f( sx,  sy, sz);
	glVertex3f(-sx,  sy, sz);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f( sx, -sy, -sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx,  sy, -sz);
	glVertex3f( sx,  sy, -sz);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(sx, -sy,  sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(sx,  sy, -sz);
	glVertex3f(sx,  sy,  sz);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, -sy,  sz);
	glVertex3f(-sx,  sy,  sz);
	glVertex3f(-sx,  sy, -sz);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-sx, sy,  sz);
	glVertex3f( sx, sy,  sz);
	glVertex3f( sx, sy, -sz);
	glVertex3f(-sx, sy, -sz);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f( sx, -sy, -sz);
	glVertex3f( sx, -sy,  sz);
	glVertex3f(-sx, -sy,  sz);

	glEnd();
}

// SPHERE ============================================
pSphere::pSphere() : Primitive(), radius(1.0f)
{
	type = PrimitiveTypes::Primitive_Sphere;
}

pSphere::pSphere(float radius) : Primitive(), radius(radius)
{
	type = PrimitiveTypes::Primitive_Sphere;
}

void pSphere::InnerRender() const
{
	//glutSolidSphere(radius, 25, 25);
}

// CYLINDER ============================================
pCylinder::pCylinder() : Primitive(), radius(1.0f), height(1.0f)
{
	type = PrimitiveTypes::Primitive_Cylinder;
}

pCylinder::pCylinder(float radius, float height) : Primitive(), radius(radius), height(height)
{
	type = PrimitiveTypes::Primitive_Cylinder;
}

void pCylinder::InnerRender() const
{
	int n = 30;

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	
	for (int i = 360; i >= 0; i -= (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians
		glVertex3f(-height * 0.5f, radius * cos(a), radius * sin(a));
	}
	glEnd();

	// Cylinder Top
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for (int i = 0; i <= 360; i += (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians
		glVertex3f(height * 0.5f, radius * cos(a), radius * sin(a));
	}
	glEnd();

	// Cylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < 480; i += (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians

		glVertex3f(height * 0.5f, radius * cos(a), radius * sin(a));
		glVertex3f(-height * 0.5f, radius * cos(a), radius * sin(a));
	}
	glEnd();
}

// LINE ==================================================
pLine::pLine() : Primitive(), origin(0.0f, 0.0f, 0.0f), destination(1.0f, 1.0f, 1.0f)
{
	type = PrimitiveTypes::Primitive_Line;
}

pLine::pLine(float x, float y, float z) : Primitive(), origin(0.0f, 0.0f, 0.0f), destination(x, y, z)
{
	type = PrimitiveTypes::Primitive_Line;
}

void pLine::InnerRender() const
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(destination.x, destination.y, destination.z);

	glEnd();

	glLineWidth(1.0f);
}

// PLANE ==================================================
pPlane::pPlane() : Primitive(), normal(0.0f, 1.0f, 0.0f), constant(1.0f)
{
	type = PrimitiveTypes::Primitive_Plane;
}

pPlane::pPlane(float x, float y, float z, float d) : Primitive(), normal(x, y, z), constant(d)
{
	type = PrimitiveTypes::Primitive_Plane;
}

void pPlane::InnerRender() const
{
	glLineWidth(1.0f);

	glBegin(GL_LINES);

	float d = 200.0f;

	for (float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glEnd();
}