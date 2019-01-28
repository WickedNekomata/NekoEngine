#include "DebugDrawer.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "glew\include\GL\glew.h"

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

void DebugDrawer::DebugDraw(const math::AABB& aabb, const Color& color, const math::float4x4& globalTransform) const
{
	static math::float3 corners[8];
	aabb.GetCornerPoints(corners);

	DebugDrawCube(corners, color, globalTransform);
}

void DebugDrawer::DebugDraw(const math::Frustum& frustum, const Color& color, const math::float4x4& globalTransform) const
{
	static math::float3 corners[8];
	frustum.GetCornerPoints(corners);

	DebugDrawCube(corners, color, globalTransform);
}

void DebugDrawer::DebugDrawCube(const math::float3* vertices, const Color& color, const math::float4x4& globalTransform) const
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

#define SPHERE_SIDES 10

void DebugDrawer::DebugDrawSphere(float radius, const Color& color, const math::float4x4& globalTransform) const
{
	glColor3f(color.r, color.g, color.b);
	glPushMatrix();
	glMultMatrixf(globalTransform.Transposed().ptr());

	float deltaAngle = 360.0f / (float)SPHERE_SIDES;

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(radius * cosf(DEGTORAD * angle), 0.0f, radius * sinf(DEGTORAD * angle));
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(radius * cosf(DEGTORAD * angle), radius * sinf(DEGTORAD * angle), 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	for (float angle = 0.0f; angle <= 360.0f; angle += deltaAngle)
		glVertex3f(0.0f, radius * sinf(DEGTORAD * angle), radius * cosf(DEGTORAD * angle));
	glEnd();

	glPopMatrix();
}