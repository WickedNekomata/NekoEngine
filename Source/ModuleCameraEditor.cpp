#include "ModuleCameraEditor.h"

#ifndef GAMEMODE

#include "Globals.h"
#include "Application.h"

#include "ComponentCamera.h"
#include "ModuleInput.h"

#include "ModuleGui.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Raycaster.h"
#include "ModuleTimeManager.h"
#include "ModuleScene.h"

#include "SDL\include\SDL_scancode.h"
#include "SDL\include\SDL_mouse.h"

#include "MathGeoLib\include\Math\MathAll.h"
#include "Brofiler\Brofiler.h"

// Reference: https://learnopengl.com/Getting-started/Camera

ModuleCameraEditor::ModuleCameraEditor(bool start_enabled) : Module(start_enabled)
{
	name = "CameraEditor";
	camera = new ComponentCamera(nullptr);
}

ModuleCameraEditor::~ModuleCameraEditor()
{
	RELEASE(camera);
}

bool ModuleCameraEditor::Init(JSON_Object* jObject)
{
	LoadStatus(jObject);

	return true;
}

bool ModuleCameraEditor::Start()
{
	bool ret = true;

	CONSOLE_LOG("Setting up the camera");

	camera->frustum.pos = { 0.0f,1.0f,-5.0f };

	return ret;
}

update_status ModuleCameraEditor::Update()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	if (!App->IsEditor() || App->gui->WantTextInput())
		return UPDATE_CONTINUE;

	// Free movement and rotation
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		// Move
		math::float3 offsetPosition(math::float3::zero);

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			offsetPosition += camera->frustum.front;
		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
			offsetPosition -= camera->frustum.front;
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			offsetPosition -= camera->frustum.WorldRight();
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			offsetPosition += camera->frustum.WorldRight();
		if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
			offsetPosition -= camera->frustum.up;
		if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
			offsetPosition += camera->frustum.up;

		float cameraMovementSpeed = movementSpeed * App->timeManager->GetRealDt();

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
			cameraMovementSpeed *= 2.0f; // double speed

		camera->frustum.Translate(offsetPosition * cameraMovementSpeed);

		// Rotate (Look Around camera position)
		int dx = -App->input->GetMouseXMotion(); // Affects the Yaw
		int dy = -App->input->GetMouseYMotion(); // Affects the Pitch

		if (dx != 0 || dy != 0)
		{
			float cameraRotationSpeed = rotationSpeed * App->timeManager->GetRealDt();

			LookAround(camera->frustum.pos, (float)dy * cameraRotationSpeed, (float)dx * cameraRotationSpeed);
		}
	}

	// Zoom
	int mouseWheel = App->input->GetMouseZ();
	if (mouseWheel != 0)
	{
		float cameraZoomSpeed = zoomSpeed * App->timeManager->GetRealDt();

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
			cameraZoomSpeed *= 0.5f; // half speed

		math::float3 offsetPosition = camera->frustum.front * (float)mouseWheel * cameraZoomSpeed;
		camera->frustum.Translate(offsetPosition);
	}

	// Look At reference
	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
		LookAt(reference, referenceRadius);

	// Look Around reference
	if ((App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT) &&
		App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		int dx = -App->input->GetMouseXMotion(); // Affects the Yaw
		int dy = -App->input->GetMouseYMotion(); // Affects the Pitch

		if (dx != 0 || dy != 0)
		{
			float cameraRotationSpeed = rotationSpeed * App->timeManager->GetRealDt();

			LookAround(reference, (float)dy * cameraRotationSpeed, (float)dx * cameraRotationSpeed);
		}
	}

	// Select game object
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN 
		&& !App->gui->IsMouseHoveringAnyWindow() && !App->scene->IsGizmoValid())
	{
		float distance;
		math::float3 hitPoint;
		GameObject* hitGameObject = nullptr;

		App->raycaster->ScreenPointToRay(App->input->GetMouseX(), App->input->GetMouseY(), distance, hitPoint, &hitGameObject);
		if (hitGameObject != nullptr)
		{
			SELECT(hitGameObject);
		}
		else
			SELECT(NULL);
	}

	return UPDATE_CONTINUE;
}

bool ModuleCameraEditor::CleanUp()
{
	CONSOLE_LOG("Cleaning camera");

	return true;
}

void ModuleCameraEditor::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_number(jObject, "movementSpeed", movementSpeed);
	json_object_set_number(jObject, "rotationSpeed", rotationSpeed);
	json_object_set_number(jObject, "zoomSpeed", zoomSpeed);
	json_object_set_number(jObject, "referenceRadius", referenceRadius);
}

void ModuleCameraEditor::LoadStatus(const JSON_Object* jObject)
{
	movementSpeed = json_object_get_number(jObject, "movementSpeed");
	rotationSpeed = json_object_get_number(jObject, "rotationSpeed");
	zoomSpeed = json_object_get_number(jObject, "zoomSpeed");
	referenceRadius = json_object_get_number(jObject, "referenceRadius");
}

void ModuleCameraEditor::SetReference(const math::float3& reference)
{
	this->reference = reference;
}

void ModuleCameraEditor::SetReferenceRadius(float referenceRadius)
{
	this->referenceRadius = referenceRadius;
}

void ModuleCameraEditor::LookAt(const math::float3& reference, float radius) const
{
	math::float3 Z = -(camera->frustum.pos - reference).Normalized(); // Direction the camera is looking at (reverse direction of what the camera is targeting)
	math::float3 X = math::Cross(math::float3(0.0f, 1.0f, 0.0f), Z).Normalized(); // X is perpendicular to vectors Y and Z
	math::float3 Y = math::Cross(Z, X); // Y is perpendicular to vectors Z and X

	camera->frustum.front = Z;
	camera->frustum.up = Y;

	if (radius != 0.0f)
	{
		float distance = (camera->frustum.pos - reference).Length();
		distance -= radius;

		camera->frustum.Translate(camera->frustum.front * distance);
	}
}

void ModuleCameraEditor::LookAround(const math::float3& reference, float pitch, float yaw) const
{
	math::Quat rotationX = math::Quat::RotateAxisAngle({ 0.0f,1.0f,0.0f }, yaw * DEGTORAD);
	math::Quat rotationY = math::Quat::RotateAxisAngle(camera->frustum.WorldRight(), pitch * DEGTORAD);
	math::Quat finalRotation = rotationX * rotationY;

	camera->frustum.up = finalRotation * camera->frustum.up;
	camera->frustum.front = finalRotation * camera->frustum.front;

	/*
	// Cap
	if (Y.y < 0.0f)
	{
		Z = math::float3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		Y = math::Cross(Z, X);
	}
	*/

	float distance = (camera->frustum.pos - reference).Length();
	camera->frustum.pos = reference + (-camera->frustum.front * distance);
}

#endif // GAME