#include "Globals.h"
#include "Application.h"
#include "ModuleCameraEditor.h"

#include "ComponentCamera.h"
#include "ModuleInput.h"

#include "ModuleGui.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Raycaster.h"

#include "MathGeoLib/include/Math/MathAll.h"

// Reference: https://learnopengl.com/Getting-started/Camera

#define MOVSPEED 5.0f
#define MOVSPEEDSHIFT 10.0f
#define ROTATIONSPEED 10.0f
#define CAMERAZOOMSPEED 10.0f;

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

	camera->cameraFrustum.pos = { 0.0f,1.0f,-5.0f };
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
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
		offsetPosition += camera->cameraFrustum.up;
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
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

		math::Quat endRotation = rotationX * rotationY;

		camera->cameraFrustum.front = endRotation * camera->cameraFrustum.front;
		camera->cameraFrustum.up = endRotation * camera->cameraFrustum.up;
	}

	int mouseWheel = App->input->GetMouseZ();
	if (mouseWheel != 0)
	{
		float zoomSpeed = CAMERAZOOMSPEED;
		math::float3 offsetPosition = math::float3::zero;
		offsetPosition += camera->cameraFrustum.front * (float)mouseWheel * zoomSpeed * dt;
		camera->cameraFrustum.Translate(offsetPosition);
	}

	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN && App->scene->currentGameObject != nullptr)
		LookAt(App->scene->currentGameObject->transform->position);

	if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT &&
		App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT && 
		App->scene->currentGameObject != nullptr)
	{ }


	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN && !App->gui->IsMouseHoveringAnyWindow())
	{
		float distance;
		math::float3 hitPoint;
		GameObject* hitGO = nullptr;
		App->raycaster->ScreenPointToRay(App->input->GetMouseX(), App->input->GetMouseY(), distance, hitPoint, &hitGO);
		App->scene->currentGameObject = hitGO;
	}

	return UPDATE_CONTINUE;
}

bool ModuleCameraEditor::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Cleaning camera");

	return ret;
}

void ModuleCameraEditor::LookAt(math::float3 focus)
{
	math::float3 direction = (camera->cameraFrustum.pos - focus).Normalized();
	
	float targetOrientation =  math::Atan2(direction.x, direction.z);
	float currentOrientation = math::Atan2(camera->cameraFrustum.front.x, camera->cameraFrustum.front.z);
	float angleToRotate = targetOrientation - currentOrientation;

	math::Quat rotation;
	rotation.SetFromAxisAngle(math::float3::unitY, angleToRotate);
	
	camera->cameraFrustum.front = rotation * camera->cameraFrustum.front;
	camera->cameraFrustum.up = rotation * camera->cameraFrustum.up;
	
	rotation.SetFromAxisAngle(math::float3::unitY, 405.0f);

	camera->cameraFrustum.front = rotation * camera->cameraFrustum.front;
	camera->cameraFrustum.up = rotation * camera->cameraFrustum.up;
	/*
	math::Quat rotationX = math::Quat::RotateAxisAngle({ 0,1,0 }, -motionX * DEGTORAD * ROTATIONSPEED * dt);
	math::Quat rotationY = math::Quat::RotateAxisAngle(camera->cameraFrustum.WorldRight(), -motionY * DEGTORAD * ROTATIONSPEED * dt);

	math::Quat endRotation = rotationX * rotationY;

	camera->cameraFrustum.front = endRotation * camera->cameraFrustum.front;
	camera->cameraFrustum.up = endRotation * camera->cameraFrustum.up;

	float my_orientation = Mathf.Rad2Deg * Mathf.Atan2(transform.forward.x, transform.forward.z);
		float target_orientation = Mathf.Rad2Deg * Mathf.Atan2(move.target.transform.forward.x, move.target.transform.forward.z);
		float diff = Mathf.DeltaAngle(my_orientation, target_orientation); // wrap around PI
		*/
}
