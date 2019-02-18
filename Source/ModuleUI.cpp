#include "ModuleUI.h"

#include "Application.h"
#include "ModuleCameraEditor.h"
#include "ComponentCamera.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ResourceShaderProgram.h"
#include "ResourceShaderObject.h"
#include "ShaderImporter.h"
#include "MaterialImporter.h"
#include "ModuleFileSystem.h"
#include "ComponentTransform.h"
#include "ComponentRect.h"
#include "ResourceTexture.h"

#include "MathGeoLib/include/Geometry/Frustum.h"

ModuleUI::ModuleUI(bool start_enabled) : Module(start_enabled)
{
	//math::Frustum::ViewportToScreenSpace();
}

ModuleUI::~ModuleUI()
{
}

void ModuleUI::DrawTest()
{
	DrawSquare(rect_test, 45.0f, { 0.0f, 1.0f, 0.0f });
}

bool ModuleUI::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleUI::Start()
{
	initRenderData();

	//Shader
	ui_vertex = new ResourceShaderObject(ResourceType::ShaderObjectResource, App->GenerateRandomNumber());
	ui_vertex->shaderType = ShaderType::VertexShaderType;
	ui_vertex->exportedFile = "Assets/Shaders/Objects/UIVertex.vsh";
	ui_vertex->LoadMemory();

	ui_fragment = new ResourceShaderObject(ResourceType::ShaderObjectResource, App->GenerateRandomNumber());
	ui_fragment->shaderType = ShaderType::FragmentShaderType;
	ui_fragment->exportedFile = "Assets/Shaders/Objects/UIFragment.fsh";
	ui_fragment->LoadMemory();

	ui_shader = new ResourceShaderProgram(ResourceType::ShaderProgramResource, App->GenerateRandomNumber());
	ui_shader->SetShaderObjects({ ui_vertex, ui_fragment });
	ui_shader->Link();
	bool program_loaded = ui_shader->IsProgramLinked();
	
	texture_test = new TextureImportSettings();
	texture_loaded = new ResourceTexture(ResourceType::TextureResource, App->GenerateRandomNumber());
	std::string uoutput_file;
	App->materialImporter->Import("Assets/Textures/Baker_house.dds", uoutput_file, texture_test);
	App->materialImporter->Load(uoutput_file.c_str(), texture_loaded, texture_test);

	rect_test = new ComponentRect(nullptr);
	rect_test->x = 200;
	rect_test->y = 200;
	rect_test->x_dist = 300;
	rect_test->y_dist = 400;

	ortho_matrix.At(0, 0) = (2 / ortho_right - ortho_left);
	ortho_matrix.At(1, 1) = (2 / ortho_top - ortho_bottom);
	ortho_matrix.At(2, 2) = (2 / ortho_far - ortho_near);

	ortho_matrix.At(0, 3) = (ortho_right + ortho_left / ortho_right - ortho_left);
	ortho_matrix.At(1, 3) = (ortho_top + ortho_bottom / ortho_top - ortho_bottom);
	ortho_matrix.At(2, 3) = (ortho_far + ortho_near / ortho_far - ortho_near);

	return true;
}

update_status ModuleUI::PreUpdate()
{
	orthonormalMatrix = math::float4x4(App->camera->camera->frustum.NearPlane().OrthoProjection());

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::Update()
{

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::FixedUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	return true;
}

void ModuleUI::OnSystemEvent(System_Event event)
{
}

void ModuleUI::initRenderData()
{
	// Configure VAO/VBO
	GLuint VBO;
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(quadVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ModuleUI::DrawSquare(ComponentRect* rect, float rotation, math::float3 color)
{
	// Prepare transformations
	/*
	uint shaderID = ui_shader->shaderProgram;
	use(shaderID);

	ComponentTransform transform(nullptr);

	transform.position = { rect->x, rect->y, 0.0f };
	transform.rotation = math::Quat({0.0f,0.0f,1.0f},rotation);

	math::float4x4 model = math::float4x4::identity;
	model = model * math::TranslateOp({ rect->x, rect->y, 0.0f });

	model = model * math::TranslateOp({ 0.5f * rect->x_dist, 0.5f * rect->y_dist, 0.0f });
	model = model * math::Quat({ 0.0f,0.0f,1.0f }, rotation);
	model = model * math::TranslateOp({ -0.5f * rect->x_dist, -0.5f * rect->y_dist, 0.0f });

	model = model * math::ScaleOp(rect->x_dist, rect->y_dist, 1.0f);
	
	setFloat4x4(shaderID, "projection", orthonormalMatrix.Transposed().ptr());
	setFloat4x4(shaderID, "model",model.Transposed().ptr());

	setFloat(shaderID, "spriteColor", color);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	*/
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	/*
	math::float4x4 ortho_matrix = math::float4x4::identity;
	ortho_matrix.At(0, 0) = (2 / );

	uint shaderID = ui_shader->shaderProgram;
	use(shaderID);
	
	setFloat4x4(shaderID, "projection", math::float4x4::identity.ptr());
	setFloat4x4(shaderID, "model", math::float4x4::identity.ptr());

	setFloat(shaderID, "spriteColor", {1.0f, 1.0f, 1.0f});

	gluOrtho2D(-100, 100, -100, 100);

	use(shaderID);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	*/
	

	uint shaderID = ui_shader->shaderProgram;
	use(shaderID);

	math::float4x4 model = math::float4x4::identity;
	model = model * math::TranslateOp({ rect->x, rect->y, 0.0f });

	model = model * math::TranslateOp({ 0.5f * rect->x_dist, 0.5f * rect->y_dist, 0.0f });
	model = model * math::Quat({ 0.0f,0.0f,1.0f }, rotation);
	model = model * math::TranslateOp({ -0.5f * rect->x_dist, -0.5f * rect->y_dist, 0.0f });

	model = model * math::ScaleOp(rect->x_dist, rect->y_dist, 1.0f);

	setFloat4x4(shaderID, "projection", ortho_matrix.ptr());
	setFloat4x4(shaderID, "model", model.ptr());
	setFloat(shaderID, "spriteColor", color);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_loaded->id);
	setInt(shaderID, "image", 0);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-100, 100, -100, 100, -1, 1);
	//gluOrtho2D(-100, 100, -100, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1, 1, 1);
	
	glBegin(GL_QUADS);
	glVertex3f(20.0f, 20.0f, 0.0f);
	glVertex3f(20.0f, -20.0f, 0.0f);
	glVertex3f(-20.0f, -20.0f, 0.0f);
	glVertex3f(-20.0f, 20.0f, 0.0f);
	glEnd();
	*/
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	/*
	//glPopMatrix();

	App->renderer3D->CalculateProjectionMatrix();
	*/
}


bool ModuleUI::GetUIMode() const
{
	return uiMode;
}

void ModuleUI::SetUIMode(bool stat)
{
	uiMode = stat;
}

// Shader methods
void ModuleUI::use(unsigned int ID)
{
	glUseProgram(ID);
}

void ModuleUI::Delete(unsigned int ID)
{
	glUseProgram(ID);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2)
{
	glUniform2i(glGetUniformLocation(ID, name), (int)value, (int)value2);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3)
{
	glUniform3i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3);
}

void ModuleUI::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4)
{
	glUniform4i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3, (int)value4);
}

void ModuleUI::setInt(unsigned int ID, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2)
{
	glUniform2i(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2, int value3)
{
	glUniform3i(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setInt(unsigned int ID, const char * name, int value, int value2, int value3, int value4)
{
	glUniform4i(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat(unsigned int ID, const char*name, float value)
{
	glUniform1f(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2)
{
	glUniform2f(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2, float value3)
{
	glUniform3f(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, float value, float value2, float value3, float value4)
{
	glUniform4f(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat(unsigned int ID, const char * name, math::float3 value)
{
	glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value)
{
	glUniform1ui(glGetUniformLocation(ID, name), value);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2)
{
	glUniform2ui(glGetUniformLocation(ID, name), value, value2);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3)
{
	glUniform3ui(glGetUniformLocation(ID, name), value, value2, value3);
}

void ModuleUI::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4)
{
	glUniform4ui(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ModuleUI::setFloat3x3(unsigned int ID, const char * name, const float * trans)
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}

void ModuleUI::setFloat4x4(unsigned int ID, const char * name, const float* trans)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}
