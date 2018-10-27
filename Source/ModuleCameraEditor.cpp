#include "Globals.h"
#include "Application.h"
#include "ModuleCameraEditor.h"

#include "ComponentCamera.h"
#include "ModuleInput.h"

// Reference: https://learnopengl.com/Getting-started/Camera

#define MOVSPEED 5.0f
#define MOVSPEEDSHIFT 10.0f
#define ROTATIONSPEED 10.0f

ModuleCameraEditor::ModuleCameraEditor(bool start_enabled) : Module(start_enabled)
{
	name = "Camera3D";

	camera = new ComponentCamera(nullptr);
}

ModuleCameraEditor::~ModuleCameraEditor()
{
	RELEASE(camera);
}

bool ModuleCameraEditor::Init(JSON_Object* jObject)
{
	return true;
}

bool ModuleCameraEditor::Start()
{
	bool ret = true;

	CONSOLE_LOG("Setting up the camera");

	camera->cameraFrustum.pos = { 0,0,0 };
	return ret;
}

update_status ModuleCameraEditor::Update(float dt)
{
	math::float3 offsetPosition = math::float3::zero;

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		offsetPosition += camera->cameraFrustum.front;
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		offsetPosition -= camera->cameraFrustum.front;
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		offsetPosition -= camera->cameraFrustum.WorldRight();
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		offsetPosition += camera->cameraFrustum.WorldRight();
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_REPEAT)
		offsetPosition += camera->cameraFrustum.up;
	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT)
		offsetPosition -= camera->cameraFrustum.up;

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		camera->cameraFrustum.Translate(offsetPosition * MOVSPEEDSHIFT * dt);
	else
		camera->cameraFrustum.Translate(offsetPosition * MOVSPEED * dt);

	int motionX = App->input->GetMouseXMotion();
	int motionY = App->input->GetMouseYMotion();

	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT && (motionX != 0 || motionY != 0))
	{
		math::Quat rotationX = math::Quat::RotateAxisAngle({ 0,1,0 }, -motionX * DEGTORAD * ROTATIONSPEED * dt);
		math::Quat rotationY = math::Quat::RotateAxisAngle(camera->cameraFrustum.WorldRight(), -motionY * DEGTORAD * ROTATIONSPEED * dt);

		math::Quat newRotation = rotationX * rotationY;
		math::float3x3 matrix = newRotation.ToFloat3x3();

		camera->cameraFrustum.front = matrix * camera->cameraFrustum.front;
		camera->cameraFrustum.up = matrix * camera->cameraFrustum.up;
	}

	return UPDATE_CONTINUE;
}

bool ModuleCameraEditor::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Cleaning camera");

	return ret;
}