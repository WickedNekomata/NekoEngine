#include "ParticlePlane.h"
#include "glew-2.1.0/include/GL/glew.h"
#include "GameObject.h"
#include "ResourceTexture.h"

// PARTICLE PLANE ==================================================
ParticlePlane::ParticlePlane()
{
	LoadPlaneBuffers();
}

ParticlePlane::~ParticlePlane()
{
	glDeleteBuffers(1, (GLuint*)&(indexID));
	glDeleteBuffers(1, (GLuint*)&(vertexID));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticlePlane::LoadPlaneBuffers()
{
	float indicesQuad[]
	{
	-0.5f, -0.5f, 0.0f,//a
	 0.5f, -0.5f, 0.0f,//b
	-0.5f,  0.5f, 0.0f,//c
	 0.5f,  0.5f, 0.0f,//d
	};

	glGenBuffers(1, (GLuint*)&(indexID));
	glBindBuffer(GL_ARRAY_BUFFER, indexID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, indicesQuad, GL_STATIC_DRAW);
	// 12 = All vertex positions (4 * 3) 4 = posibleVertex and 3 = pos x-y-z
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	uint vertices[]
	{
		// Front
		0, 1, 2, // ABC
		1, 3, 2, // BDC
	};

	glGenBuffers(1, (GLuint*)&(vertexID));
	glBindBuffer(GL_ARRAY_BUFFER, vertexID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint) * 6, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticlePlane::Render(float4x4 matrix, ResourceTexture* texture, uint textureUV, float4 color) const
{
	glPushMatrix();
	
	glMultMatrixf(matrix.ptr());

	DrawPlane(texture, textureUV, color);

	glPopMatrix();
}

void ParticlePlane::DrawPlane(ResourceTexture* texture,uint textureUV, math::float4 &color) const
{
	glEnableClientState(GL_VERTEX_ARRAY);

	//Load vertex and index
	glBindBuffer(GL_ARRAY_BUFFER, indexID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (texture != nullptr)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Load Texture UV
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, textureUV);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);

		//glColor4f(color.x, color.y, color.z, color.w);
		glEnable(GL_ALPHA_TEST);

		//Load texture
		glBindTexture(GL_TEXTURE_2D, texture->GetID());
	}


	glColor4f(color.x, color.y, color.z, color.w);
	//Draw mesh
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	//Free buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
