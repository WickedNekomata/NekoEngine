#include "DebugDrawer.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "glew/include/GL/glew.h"

DebugDrawer::DebugDrawer() {}

DebugDrawer::~DebugDrawer() {}

void DebugDrawer::StartDebugDraw()
{
	cullFace = App->renderer3D->GetCapabilityState(GL_CULL_FACE);
	lighting = App->renderer3D->GetCapabilityState(GL_LIGHTING);
	texture2D = App->renderer3D->GetCapabilityState(GL_TEXTURE_2D);
	wireframeMode = App->renderer3D->IsWireframeMode();

	App->renderer3D->SetCapabilityState(GL_CULL_FACE, false);
	App->renderer3D->SetCapabilityState(GL_LIGHTING, false);
	App->renderer3D->SetCapabilityState(GL_TEXTURE_2D, false);
	App->renderer3D->SetWireframeMode(true);
}

void DebugDrawer::EndDebugDraw()
{
	App->renderer3D->SetCapabilityState(GL_CULL_FACE, cullFace);
	App->renderer3D->SetCapabilityState(GL_LIGHTING, lighting);
	App->renderer3D->SetCapabilityState(GL_TEXTURE_2D, texture2D);
	App->renderer3D->SetWireframeMode(wireframeMode);
}

void DebugDrawer::DebugDraw(const math::AABB& aabb, Color color, const math::float4x4 globalTransform) const
{
	static math::float3 corners[8];
	aabb.GetCornerPoints(corners);

	DebugDrawCube(corners, color, globalTransform);
}

void DebugDrawer::DebugDraw(const math::Frustum& frustum, Color color, const math::float4x4 globalTransform) const
{
	static math::float3 corners[8];
	frustum.GetCornerPoints(corners);

	DebugDrawCube(corners, color, globalTransform);
}

void DebugDrawer::DebugDrawCube(const math::float3* vertices, Color color, const math::float4x4 globalTransform) const
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	glBegin(GL_QUADS);

	glVertex3fv((const GLfloat*)&vertices[1]);
	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[7]);
	glVertex3fv((const GLfloat*)&vertices[3]);

	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[2]);
	glVertex3fv((const GLfloat*)&vertices[6]);

	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[6]);
	glVertex3fv((const GLfloat*)&vertices[7]);

	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[1]);
	glVertex3fv((const GLfloat*)&vertices[3]);
	glVertex3fv((const GLfloat*)&vertices[2]);

	glVertex3fv((const GLfloat*)&vertices[3]);
	glVertex3fv((const GLfloat*)&vertices[7]);
	glVertex3fv((const GLfloat*)&vertices[6]);
	glVertex3fv((const GLfloat*)&vertices[2]);

	glVertex3fv((const GLfloat*)&vertices[0]);
	glVertex3fv((const GLfloat*)&vertices[4]);
	glVertex3fv((const GLfloat*)&vertices[5]);
	glVertex3fv((const GLfloat*)&vertices[1]);

	glEnd();

	glPopMatrix();
}