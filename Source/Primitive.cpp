#include "Globals.h"
#include "glew/include/GL/glew.h"
#include "Primitive.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

Primitive::Primitive() : type(PrimitiveTypes::PrimitiveNoType) {}

void Primitive::Render() const
{
	InnerRender();
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

PrimitiveCube::PrimitiveCube(math::float3 position, math::float3 size) : Primitive(), size(size)
{
	type = PrimitiveTypes::PrimitiveTypeCube;

	math::float3 radius = size / 2.0f;

	vertices = new GLfloat[24]{

		-radius.x, -radius.y, radius.z, // A (0)
		radius.x,  -radius.y, radius.z, // B (1)
		-radius.x, radius.y,  radius.z, // C (2)
		radius.x,  radius.y,  radius.z, // D (3)
		-radius.x, -radius.y, -radius.z, // E (4)
		radius.x,  -radius.y, -radius.z, // F (5)
		-radius.x, radius.y,  -radius.z, // G (6)
		radius.x,  radius.y,  -radius.z  // H (7)
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * 24, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLubyte indices[36] {

		// Front
		0, 1, 2, // ABC
		1, 3, 2, // BDC

		// Right
		1, 5, 3, // BFD
		5, 7, 3, // FHD

		// Back
		5, 4, 7, // FEH
		4, 6, 7, // EGH

		// Left
		4, 0, 6, // EAG
		0, 2, 6, // ACG

		// Top
		2, 3, 6, // CDG
		3, 7, 6, // DHG

		// Bottom
		0, 4, 1, // AEB
		1, 4, 5  // BEF
	};

	indicesSize = sizeof(indices) / sizeof(GLubyte);

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 36, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

PrimitiveCube::~PrimitiveCube() 
{
	delete[] vertices;
}

void PrimitiveCube::InnerRender() const
{	
	// Vertex Array with indices (Draw Elements)
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
}

math::float3 PrimitiveCube::GetSize() const 
{
	return size;
}

PrimitiveRay::PrimitiveRay(math::float3 startPos, math::float3 endPos)
{
	vertices = new GLfloat[6]
	{
		startPos.x, startPos.y, startPos.z,
		endPos.x, endPos.y, endPos.z
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * 6, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

PrimitiveRay::~PrimitiveRay()
{
	delete[] vertices;
}

void PrimitiveRay::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_LINES, 0, 2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
}

PrimitivePlane::PrimitivePlane(math::float3 position, math::float3 size)
{
	math::float3 radius = size / 2.0f;

	vertices = new GLfloat[12]
	{
	   	 radius.x, position.y, -radius.z, // A
		 radius.x, position.y,  radius.z, // B
		-radius.x, position.y, -radius.z, // C
		-radius.x, position.y,  radius.z, // D
	};

	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * 12, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLubyte indices[6]{
		2, 1, 0,
		3, 1, 2
	};

	indicesSize = sizeof(indices) / sizeof(GLubyte);

	glGenBuffers(1, &indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

PrimitivePlane::~PrimitivePlane()
{
	delete[] vertices;
}

void PrimitivePlane::InnerRender() const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
}
