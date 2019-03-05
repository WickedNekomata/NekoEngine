#include "ModuleFBOManager.h"
#include "glew\include\GL\glew.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ResourceShaderProgram.h"
#include "ModuleInternalResHandler.h"

ModuleFBOManager::ModuleFBOManager() {}

ModuleFBOManager::~ModuleFBOManager() {}

bool ModuleFBOManager::Start()
{
	LoadGBuffer();
	return true;
}

bool ModuleFBOManager::CleanUp()
{
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteRenderbuffers(1, &rboDepth);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedoSpec);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void ModuleFBOManager::LoadGBuffer()
{
	// DEFERRED SHADING G BUFFER
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	uint width = App->window->GetWindowWidth();
	uint height = App->window->GetWindowHeight();
	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// then also add render buffer object as depth buffer and check for completeness.
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		CONSOLE_LOG(LogTypes::Error, "Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleFBOManager::BindGBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ModuleFBOManager::DrawGBufferToScreen() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ResourceShaderProgram* resProgram;// = (ResourceShaderProgram*)App->res->GetResource(App->resHandler->deferredShaderProgram);
	glUseProgram(resProgram->shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	uint location = glGetUniformLocation(resProgram->shaderProgram, "gPosition");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	location = glGetUniformLocation(resProgram->shaderProgram, "gNormal");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	location = glGetUniformLocation(resProgram->shaderProgram, "gAlbedoSpec");
	glUniform1i(location, 2);

	const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(App->resHandler->plane);

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
	glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void ModuleFBOManager::MergeDepthBuffer()
{
	// Here we write current depth buffer from g buffer to default
	// buffer so we can draw in forward rendering as we used to
	uint width = App->window->GetWindowWidth();
	uint height = App->window->GetWindowHeight();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	glBlitFramebuffer(
		0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
}
