#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"

#define CAMERA_MOVEMENT_SPEED 5.0f
#define CAMERA_ROTATE_SENSITIVITY 0.1f
#define CAMERA_ZOOM_SPEED 100.0f
#define CAMERA_ORBIT_SPEED 1.0f

// Reference: https://learnopengl.com/Getting-started/Camera

ModuleCamera3D::ModuleCamera3D(bool start_enabled) : Module(start_enabled)
{
	name = "Camera3D";

	// Right of the camera (WORLD SPACE)
	X = math::float3(1.0f, 0.0f, 0.0f);
	// Up of the camera (WORLD SPACE)
	Y = math::float3(0.0f, 1.0f, 0.0f);
	// Direction the camera is looking at (reverse direction of what the camera is targeting) (WORLD SPACE)
	Z = math::float3(0.0f, 0.0f, 1.0f);

	// Position of the camera (WORLD SPACE)
	position = math::float3(0.0f, 0.0f, 0.0f);
	// Target of the camera (WORLD SPACE)
	reference = math::float3(0.0f, 0.0f, 0.0f);

	referenceRadius = 1.0f;

	CalculateViewMatrix();
}

ModuleCamera3D::~ModuleCamera3D()
{}

bool ModuleCamera3D::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleCamera3D::Start()
{
	bool ret = true;

	CONSOLE_LOG("Setting up the camera");

	return ret;
}

update_status ModuleCamera3D::Update(float dt)
{
	// Free movement and rotation
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		// Move
		float cameraSpeed = CAMERA_MOVEMENT_SPEED * dt;

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
			cameraSpeed *= 2.0f; // double speed

		math::float3 newPosition(0.0f, 0.0f, 0.0f);

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			newPosition -= Z * cameraSpeed;
		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
			newPosition += Z * cameraSpeed;
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			newPosition -= X * cameraSpeed;
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			newPosition += X * cameraSpeed;
		if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
			newPosition -= math::float3(0.0f, 1.0f, 0.0f) * cameraSpeed;
		if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
			newPosition += math::float3(0.0f, 1.0f, 0.0f) * cameraSpeed;

		Move(newPosition);

		// Look Around (camera position)
		int dx = -App->input->GetMouseXMotion(); // Affects the Yaw
		int dy = -App->input->GetMouseYMotion(); // Affects the Pitch

		float rotateSensitivity = CAMERA_ROTATE_SENSITIVITY;
		float deltaX = (float)dx * rotateSensitivity * dt;
		float deltaY = (float)dy * rotateSensitivity * dt;

		math::float3 target = position;
		LookAround(target, deltaY, deltaX);
	}

	// Zoom
	int mouseWheel = App->input->GetMouseZ();
	if (mouseWheel != 0)
	{
		float zoomSpeed = CAMERA_ZOOM_SPEED;
		float zoom = (float)mouseWheel * zoomSpeed * dt;

		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT)
			zoom *= 0.5f; // half speed

		Zoom(zoom);
	}

	// Look At target
	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
		LookAt(reference, referenceRadius);

	// Look Around target
	if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RALT) == KEY_REPEAT)
	{
		if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
		{
			// Look Around (target position)
			int dx = -App->input->GetMouseXMotion(); // Affects the Yaw
			int dy = -App->input->GetMouseYMotion(); // Affects the Pitch

			float rotateSensitivity = CAMERA_ROTATE_SENSITIVITY;
			float deltaX = (float)dx * rotateSensitivity * dt;
			float deltaY = (float)dy * rotateSensitivity * dt;

			LookAround(reference, deltaY, deltaX);
		}
	}

	/*
	
	if (isOrbiting)
		// Set the radius distance here (once)
		LookAt(reference, radius);

	if (isOrbiting)
	{
		// Ignore the radius here, in order to let the user zoom in/out while orbiting
		Orbit(reference, dt, orbitSpeed);
	}
	*/

	// Orbit target
	if (orbit)
	{
		// Update position
		float orbitSpeed = CAMERA_ORBIT_SPEED;
		math::float3 newPosition = X * orbitSpeed * dt;
		Move(newPosition);

		// Update Look At
		LookAt(reference, referenceRadius);
	}

	return UPDATE_CONTINUE;
}

bool ModuleCamera3D::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Cleaning camera");

	return ret;
}

void ModuleCamera3D::SetTarget(math::float3 target)
{
	reference = target;
}

void ModuleCamera3D::SetTargetRadius(float targetRadius)
{
	referenceRadius = targetRadius;
}

// Creates a View Matrix that looks at the given target
void ModuleCamera3D::LookAt(const math::float3 &reference, float radius)
{
	Z = (position - reference).Normalized(); // Direction the camera is looking at (reverse direction of what the camera is targeting)
	X = math::Cross(math::float3(0.0f, 1.0f, 0.0f), Z).Normalized(); // X is perpendicular to vectors Y and Z
	Y = math::Cross(Z, X); // Y is perpendicular to vectors Z and X

	if (radius != 0.0f)
	{
		float distanceTarget = math::Distance(position, reference) - radius;

		math::float3 moveDistance = -Z * distanceTarget;
		Move(moveDistance);
	}

	CalculateViewMatrix();
}

// Creates a View Matrix that looks around the given target
void ModuleCamera3D::LookAround(const math::float3 &reference, float pitch, float yaw)
{
	position -= reference;

	// Yaw (Y axis)
	if (yaw != 0.0f)
	{
		math::float3x3 rotationMatrix = math::float3x3::RotateY(yaw); // Y in world coordinates
		X = rotationMatrix * X;
		Y = rotationMatrix * Y;
		Z = rotationMatrix * Z;
	}

	// Pitch (X axis)
	if (pitch != 0.0f)
	{
		math::float3x3 rotationMatrix = math::float3x3::RotateAxisAngle(X, pitch); // X in local coordinates
		Y = rotationMatrix * Y;
		Z = rotationMatrix * Z;

		/*
		// Cap
		if (Y.y < 0.0f)
		{
			Z = math::float3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
			Y = math::Cross(Z, X);
		}
		*/
	}

	position = reference + (Z * position.Length());

	CalculateViewMatrix();
}

void ModuleCamera3D::Move(const math::float3 &movement)
{
	position += movement;

	CalculateViewMatrix();
}

void ModuleCamera3D::MoveTo(const math::float3 &position)
{
	this->position = position;

	CalculateViewMatrix();
}

void ModuleCamera3D::Zoom(float zoom)
{
	math::float3 zoomDistance = -Z * zoom;
	Move(zoomDistance);
}

float* ModuleCamera3D::GetViewMatrix()
{
	return ViewMatrix.ptr();
}

void ModuleCamera3D::CalculateViewMatrix()
{
	// We move the entire scene around inversed to where we want the camera to move
	ViewMatrix = math::float4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -math::Dot(X, position), -math::Dot(Y, position), -math::Dot(Z, position), 1.0f);
	ViewMatrixInverse = ViewMatrix.Inverted();
}