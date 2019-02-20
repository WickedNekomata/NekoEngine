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
#include "ModuleWindow.h"

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
	DrawUI(rect_test, 0.0f, { 0.0f, 1.0f, 0.0f });
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
	rect_test->x = 0;
	rect_test->y = 0;
	rect_test->x_dist = 100;
	rect_test->y_dist = 100;

	return true;
}

update_status ModuleUI::PreUpdate()
{
	//orthonormalMatrix = math::float4x4(App->camera->camera->frustum.NearPlane().OrthoProjection());

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
		// Pos      
		1.0f,  1.0f, 
		1.0f, -1.0f, 
		-1.0f,  1.0f,

		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f
	};

	glGenVertexArrays(1, &reference_vertex);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glBindVertexArray(reference_vertex);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ModuleUI::DrawUI(ComponentRect* rect, float rotation, math::float3 color)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	uint shaderID = ui_shader->shaderProgram;
	use(shaderID);

	float w_width = App->window->GetWindowWidth();
	float w_height = App->window->GetWindowHeight();

	math::float2 pos;
	pos = math::Frustum::ScreenToViewportSpace({ rect->x, rect->y }, w_width, w_height);
	setFloat(shaderID, "topLeft", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ rect->x + rect->x_dist, rect->y }, w_width, w_height);
	setFloat(shaderID, "topRight", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ rect->x, rect->y + rect->y_dist }, w_width, w_height);
	setFloat(shaderID, "bottomLeft", pos.x, pos.y);
	pos = math::Frustum::ScreenToViewportSpace({ rect->x + rect->x_dist, rect->y + rect->y_dist }, w_width, w_height);
	setFloat(shaderID, "bottomRight", pos.x, pos.y);

	setFloat(shaderID, "spriteColor", color);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture_loaded->id);
	//setInt(shaderID, "image", 0);

	glBindVertexArray(reference_vertex);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
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
