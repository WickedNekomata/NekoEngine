#include "Application.h"
#include "Particle.h"
#include "Primitive.h"
#include "ComponentEmitter.h"
#include "ModuleParticles.h"
#include "ModuleRenderer3D.h"
#include "ShaderImporter.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ComponentMaterial.h"

#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

//#include "pcg-c-basic-0.9/pcg_basic.h"

Particle::Particle(math::float3 pos, StartValues data, ResourceTexture** texture)
{}

Particle::Particle()
{

}

Particle::~Particle()
{
	//delete plane;
}

void Particle::SetActive(math::float3 pos, StartValues data, ResourceTexture ** texture, /*std::vector<uint>* animation,*/ float animationSpeed)
{
	color.clear();
	plane = App->particle->plane;

	lifeTime = CreateRandomNum(data.life);

	life = 0.0f;

	speed = CreateRandomNum(data.speed);
	acceleration = CreateRandomNum(data.acceleration);
	direction = data.particleDirection;

	angle = CreateRandomNum(data.rotation) * DEGTORAD;
	angularVelocity = CreateRandomNum(data.angularVelocity) * DEGTORAD;
	angularAcceleration = CreateRandomNum(data.angularAcceleration) * DEGTORAD;

	sizeOverTime = CreateRandomNum(data.sizeOverTime);

	transform.position = pos;
	transform.rotation = math::Quat::FromEulerXYZ(0, 0, 0); //Start rotation
	transform.scale = math::float3::one * CreateRandomNum(data.size);

	//LOG("life %f", lifeTime);
	//LOG("size %f", transform.scale.x);

	for (std::list<ColorTime>::iterator iter = data.color.begin(); iter != data.color.end(); ++iter)
		color.push_back(*iter);

	multicolor = data.timeColor;
	this->texture = texture;


	//TODO Particle: Active Particle
	this->animation = animation;
	this->animationSpeed = animationSpeed;
	animationTime = 0.0f;
	currentFrame = 0u;

	active = true;
	subEmitterActive = data.subEmitterActive;
	index = 0;

	App->particle->activeParticles++;
}

bool Particle::Update(float dt)
{
	//BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	if (owner->simulatedGame == SimulatedGame_PAUSE || App->IsPause())
		dt = 0;
	life += dt;
	if (life < lifeTime || owner->dieOnAnimation)
	{
		speed += acceleration * dt;
		transform.position += direction * (speed * dt);
		LookAtCamera();

		if (color.size() == 1 || !multicolor)
			currentColor = color.front().color;

		else if (index + 1 < color.size())
		{
			float lifeNormalized = life / lifeTime;
			if (color[index + 1].position > lifeNormalized)
			{
				float timeNormalized = (lifeNormalized - color[index].position) / (color[index + 1].position - color[index].position);
				if (color[index + 1].position == 0)
					timeNormalized = 0;
				//LOG("%i", index);
				currentColor = color[index].color.Lerp(color[index + 1].color, timeNormalized);
				//LERP Color
			}
			else
				index++;
		}
		else
			currentColor = color[index].color;

		transform.scale.x += sizeOverTime * dt;
		transform.scale.y += sizeOverTime * dt;
		transform.scale.z += sizeOverTime * dt;

		angularVelocity += angularAcceleration * dt;
		angle += angularVelocity * dt;
		transform.rotation = transform.rotation.Mul(math::Quat::RotateZ(angle));

		if (animation)
		{
			animationTime += dt;
			if (animationTime > animationSpeed)
			{
				if (animation->size() > currentFrame + 1)
				{
					currentFrame++;
				}
				else if (owner->dieOnAnimation)
				{
					EndParticle();
				}
				else
					currentFrame = 0;

				animationTime = 0.0f;
			}
		}
	}
	else
	{
		EndParticle();
	}

	return true;
}

void Particle::EndParticle()
{
	//BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
	active = false;
	owner->particles.remove(this);
	App->particle->activeParticles--;
	if (owner->subEmitter)
	{
		ComponentEmitter* emitter = (ComponentEmitter*)owner->subEmitter->GetComponentByType(EmitterComponent);
		if (subEmitterActive && emitter)
		{
			math::float3 globalPos = owner->subEmitter->transform->position;
			emitter->newPositions.push_back(transform.position - globalPos);
		}
	}
}

void Particle::LookAtCamera()
{
	math::float3 zAxis = -App->renderer3D->GetCurrentCamera()->frustum.front;
	math::float3 yAxis = App->renderer3D->GetCurrentCamera()->frustum.up;
	math::float3 xAxis = yAxis.Cross(zAxis).Normalized();

	transform.rotation.Set(math::float3x3(xAxis, yAxis, zAxis));
}

float Particle::GetCamDistance() const
{
	return App->renderer3D->GetCurrentCamera()->frustum.pos.DistanceSq(transform.position);
}

void Particle::SetCamDistance()
{
	 camDistance = App->renderer3D->GetCurrentCamera()->frustum.pos.DistanceSq(transform.position);
}

void Particle::Draw()
{
	if (active)
	{
		// Shader
		GLuint shaderProgram = App->shaderImporter->GetDefaultShaderProgram();

		glUseProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, owner->material->res[0].id); // particle texture

		glUniform1i(glGetUniformLocation(shaderProgram, "material.albedo"), 0);
		glUniform1i(glGetUniformLocation(shaderProgram, "material.specular"), 0);
		glUniform1i(glGetUniformLocation(shaderProgram, "material.normalMap"), 0);
		
		math::float4x4 model_matrix = transform.GetMatrix();// particle matrix
		model_matrix = model_matrix.Transposed();
		math::float4x4 view_matrix = App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix();
		math::float4x4 proj_matrix = App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix();
		math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
		math::float4x4 normal_matrix = model_matrix;
		normal_matrix.Inverse();
		normal_matrix.Transpose();

		uint location = glGetUniformLocation(shaderProgram, "model_matrix");
		glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
		location = glGetUniformLocation(shaderProgram, "mvp_matrix");
		glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
		location = glGetUniformLocation(shaderProgram, "normal_matrix");
		glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());

		location = glGetUniformLocation(shaderProgram, "light.direction");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.direction.ptr());
		location = glGetUniformLocation(shaderProgram, "light.ambient");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.ambient.ptr());
		location = glGetUniformLocation(shaderProgram, "light.diffuse");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.diffuse.ptr());
		location = glGetUniformLocation(shaderProgram, "light.specular");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.specular.ptr());

		uint defaultPlaneVAO = 0;
		uint defaultPlaneIBO = 0;
		uint defaultPlaneIndicesSize = 0;
		App->sceneImporter->GetDefaultPlane(defaultPlaneVAO, defaultPlaneIBO, defaultPlaneIndicesSize);

		glBindVertexArray(defaultPlaneVAO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultPlaneIBO);
		glDrawElements(GL_TRIANGLES, defaultPlaneIndicesSize, GL_UNSIGNED_INT, NULL);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}

float Particle::CreateRandomNum(math::float2 edges)//.x = minPoint & .y = maxPoint
{
	float num = edges.x;
	if (edges.x < edges.y)
	{
		float random = App->GenerateRandomNumber();	
		num = ((edges.y - edges.x) * random / (float)MAXUINT) + edges.x;
	}
	return num;
}

//Particle transform
math::float4x4 ParticleTrans::GetMatrix() const
{
	return  math::float4x4::FromTRS(position, rotation, scale);
}
