#include "Application.h"
#include "Particle.h"
#include "Primitive.h"
#include "ComponentEmitter.h"
#include "ModuleParticles.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ShaderImporter.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ComponentMaterial.h"
#include "ResourceShaderProgram.h"
#include "ResourceMaterial.h"
#include "Uniforms.h"

#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"

#include <vector>

//#include "pcg-c-basic-0.9/pcg_basic.h"

Particle::Particle(math::float3 pos, StartValues data)
{}

Particle::Particle()
{

}

Particle::~Particle()
{
}

void Particle::SetActive(math::float3 pos, StartValues data, ParticleAnimation partAnim)
{
	color.clear();

	lifeTime = CreateRandomNum(data.life);

	life = 0.0f;

	speed = CreateRandomNum(data.speed);
	acceleration3 = data.acceleration3;
	direction = data.particleDirection;

	angle = CreateRandomNum(data.rotation) * DEGTORAD;
	angularVelocity = CreateRandomNum(data.angularVelocity) * DEGTORAD;
	angularAcceleration = CreateRandomNum(data.angularAcceleration) * DEGTORAD;

	sizeOverTime = CreateRandomNum(data.sizeOverTime);

	transform.position = pos;
	transform.rotation = math::Quat::FromEulerXYZ(0, 0, 0); //Start rotation
	transform.scale = math::float3::one * CreateRandomNum(data.size);

	for (std::list<ColorTime>::iterator iter = data.color.begin(); iter != data.color.end(); ++iter)
		color.push_back(*iter);

	multicolor = data.timeColor;

	animationTime = 0.0f;
	currentFrame = 0u;

	isParticleAnimated = partAnim.isParticleAnimated;
	textureRows = partAnim.textureRows;
	textureColumns = partAnim.textureColumns;
	textureRowsNorm = partAnim.textureRowsNorm;
	textureColumnsNorm = partAnim.textureColumnsNorm;
	animationSpeed = partAnim.animationSpeed;

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
		acceleration3 += acceleration3 * dt;
		math::float3 movement = direction * (speed * dt);

		if(acceleration3.Equals(math::float3::zero))
			transform.position +=  movement;
		else
			transform.position += (movement + acceleration3 * dt)/2;

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

		if (isParticleAnimated)
		{
			animationTime += dt;
			if (animationTime > animationSpeed)
			{
				if ((textureColumns* textureRows) >= currentFrame + 1)
				{
					currentFrame++;

					currMinUVCoord.x = (currentFrame % textureColumns) * textureColumnsNorm;
					currMinUVCoord.y = (currentFrame / textureColumns) * textureRowsNorm;
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
		ComponentEmitter* emitter = (ComponentEmitter*)owner->subEmitter->GetComponent(EmitterComponent);
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
		ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(owner->materialRes);
		uint shaderUuid = resourceMaterial->GetShaderUuid();
		ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderUuid);
		GLuint shaderProgram = resourceShaderProgram->shaderProgram;

		glUseProgram(shaderProgram);
		
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
		location = glGetUniformLocation(shaderProgram, "currColor");
		glUniform4f(location,currentColor.x, currentColor.y, currentColor.z, currentColor.w);

		location = glGetUniformLocation(shaderProgram, "rowUVNorm");
		glUniform1f(location, textureRowsNorm);
		location = glGetUniformLocation(shaderProgram, "columUVNorm");
		glUniform1f(location, textureColumnsNorm);
		location = glGetUniformLocation(shaderProgram, "currMinCoord");
		glUniform2f(location, currMinUVCoord.x, currMinUVCoord.y);
		location = glGetUniformLocation(shaderProgram, "isAnimated");
		glUniform1i(location, isParticleAnimated);

		location = glGetUniformLocation(shaderProgram, "light.direction");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.direction.ptr());
		location = glGetUniformLocation(shaderProgram, "light.ambient");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.ambient.ptr());
		location = glGetUniformLocation(shaderProgram, "light.diffuse");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.diffuse.ptr());
		location = glGetUniformLocation(shaderProgram, "light.specular");
		glUniform3fv(location, 1, App->renderer3D->directionalLight.specular.ptr());

		// Unknown uniforms
		uint textureUnit = 0;
		std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
		for (uint i = 0; i < uniforms.size(); ++i)
		{
			Uniform uniform = uniforms[i];

			if (strcmp(uniform.common.name, "averageColor") == 0 || strcmp(uniform.common.name, "material.albedo") == 0 || strcmp(uniform.common.name, "material.specular") == 0)
			{
				switch (uniform.common.type)
				{
				case Uniforms_Values::FloatU_value:
					glUniform1f(uniform.common.location, uniform.floatU.value);
					break;
				case Uniforms_Values::IntU_value:
					glUniform1i(uniform.common.location, uniform.intU.value);
					break;
				case Uniforms_Values::Vec2FU_value:
					glUniform2f(uniform.common.location, uniform.vec2FU.value.x, uniform.vec2FU.value.y);
					break;
				case Uniforms_Values::Vec3FU_value:
					glUniform3f(uniform.common.location, uniform.vec3FU.value.x, uniform.vec3FU.value.y, uniform.vec3FU.value.z);
					break;
				case Uniforms_Values::Vec4FU_value:
					glUniform4f(uniform.common.location, uniform.vec4FU.value.x, uniform.vec4FU.value.y, uniform.vec4FU.value.z, uniform.vec4FU.value.w);
					break;
				case Uniforms_Values::Vec2IU_value:
					glUniform2i(uniform.common.location, uniform.vec2IU.value.x, uniform.vec2IU.value.y);
					break;
				case Uniforms_Values::Vec3IU_value:
					glUniform3i(uniform.common.location, uniform.vec3IU.value.x, uniform.vec3IU.value.y, uniform.vec3IU.value.z);
					break;
				case Uniforms_Values::Vec4IU_value:
					glUniform4i(uniform.common.location, uniform.vec4IU.value.x, uniform.vec4IU.value.y, uniform.vec4IU.value.z, uniform.vec4IU.value.w);
					break;
				case Uniforms_Values::Sampler2U_value:
					if (textureUnit < App->renderer3D->GetMaxTextureUnits())
					{
						glActiveTexture(GL_TEXTURE0 + textureUnit);
						glBindTexture(GL_TEXTURE_2D, uniform.sampler2DU.value.id);
						glUniform1i(uniform.common.location, textureUnit);
						++textureUnit;
					}
					break;
				}
			}
		}

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

void Particle::ChangeAnim(ParticleAnimation partAnim)
{
	currentFrame = 0u;
	isParticleAnimated = partAnim.isParticleAnimated;
	textureRows = partAnim.textureRows;
	textureColumns = partAnim.textureColumns;
	textureRowsNorm = partAnim.textureRowsNorm;
	textureColumnsNorm = partAnim.textureColumnsNorm;
	animationSpeed = partAnim.animationSpeed;
}

//Particle transform
math::float4x4 ParticleTrans::GetMatrix() const
{
	return  math::float4x4::FromTRS(position, rotation, scale);
}
