#include "Application.h"

#include "ModuleGOs.h"
#include "ComponentEmitter.h"
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleScene.h"
#include "ResourceTexture.h"
#include "ModuleResourceManager.h"

#include "ComponentMaterial.h"

#include <vector>

#include "ModuleParticles.h"
#include "imgui\imgui.h"

ComponentEmitter::ComponentEmitter(GameObject* gameObject) : Component(gameObject, EmitterComponent)
{
	App->scene->quadtree.Insert(gameObject);
	App->particle->emitters.push_back(this);

	//material = (ComponentMaterial*)parent->AddComponent(ComponentTypes::MaterialComponent);

}

ComponentEmitter::ComponentEmitter(const ComponentEmitter& componentEmitter) : Component(componentEmitter.parent, EmitterComponent)
{
	duration = componentEmitter.duration;

	loop = componentEmitter.loop;

	burst = componentEmitter.burst;
	minPart = componentEmitter.minPart;
	maxPart = componentEmitter.maxPart;
	repeatTime = componentEmitter.repeatTime;

	// posDifAABB
	posDifAABB = componentEmitter.posDifAABB;
	gravity = componentEmitter.gravity;

	// boxCreation
	boxCreation = componentEmitter.boxCreation;
	// SphereCreation
	sphereCreation.r = componentEmitter.sphereCreation.r;

	circleCreation.r = componentEmitter.circleCreation.r;

	normalShapeType = componentEmitter.normalShapeType;

	startValues = componentEmitter.startValues;

	checkLife = componentEmitter.checkLife;
	checkSpeed = componentEmitter.checkSpeed;
	checkAcceleration = componentEmitter.checkAcceleration;
	checkSize = componentEmitter.checkSize;
	checkRotation = componentEmitter.checkRotation;
	checkAngularAcceleration = componentEmitter.checkAngularAcceleration;
	checkSizeOverTime = componentEmitter.checkSizeOverTime;
	checkAngularVelocity = componentEmitter.checkAngularVelocity;

	isParticleAnimated = componentEmitter.isParticleAnimated;
	if (isParticleAnimated)
	{
		animationSpeed = componentEmitter.animationSpeed;
		textureRows = componentEmitter.textureRows;
		textureColumns = componentEmitter.textureColumns;
	}

	dieOnAnimation = componentEmitter.dieOnAnimation;

	drawAABB = componentEmitter.drawAABB;

	isSubEmitter = componentEmitter.isSubEmitter;
	subEmitter = componentEmitter.subEmitter;
	subEmitterUUID = componentEmitter.subEmitterUUID;

	rateOverTime = componentEmitter.rateOverTime;

	if (parent)
		App->scene->quadtree.Insert(parent);

	App->particle->emitters.push_back(this);

	//material = (ComponentMaterial*)parent->AddComponent(ComponentTypes::MaterialComponent);
}


ComponentEmitter::~ComponentEmitter()
{

	App->timeManager->RemoveGameTimer(&timer);
	App->timeManager->RemoveGameTimer(&burstTime);
	App->timeManager->RemoveGameTimer(&loopTimer);
	App->timeManager->RemoveGameTimer(&timeSimulating);

	App->particle->RemoveEmitter(this);

	ClearEmitter();
}

void ComponentEmitter::StartEmitter()
{
	if (!isSubEmitter)
	{
		timer.Start();
		burstTime.Start();
		loopTimer.Start();

		timeToParticle = 0.0f;
	}
}

void ComponentEmitter::ChangeGameState(SimulatedGame state)
{
	simulatedGame = state;
	if (state == SimulatedGame_PLAY)
		state = SimulatedGame_STOP;
	else if (state == SimulatedGame_STOP)
		ClearEmitter();

	if (subEmitter)
	{
		ComponentEmitter* compEmitter = (ComponentEmitter*)(subEmitter->GetComponent(EmitterComponent));
		if (compEmitter)
			compEmitter->ChangeGameState(state);
	}
}

void ComponentEmitter::Update()
{
	if (rateOverTime > 0)
	{
		float time = timer.ReadSec();
		if (time > timeToParticle && (loop || loopTimer.ReadSec() < duration))
		{
 			if (App->IsPlay() || simulatedGame == SimulatedGame_PLAY || App->IsStep())
			{
				int particlesToCreate = (time / (1.0f / rateOverTime));
				CreateParticles(particlesToCreate, normalShapeType,math::float3::zero);
				timeToParticle = (1.0f / rateOverTime);
				
				timer.Start();
			}

		}
	}
	float burstT = burstTime.ReadSec();
	if (burst && burstT > repeatTime)
	{
		if (App->IsPlay() || simulatedGame == SimulatedGame_PLAY || App->IsStep())
		{
			int particlesToCreate = minPart;
			if (minPart != maxPart)
				particlesToCreate = (rand() % (maxPart - minPart)) + minPart;
			CreateParticles(particlesToCreate, burstType, math::float3::zero);
		}
		burstTime.Start();
	}

	//Used for SubEmitter. Create particles from ParticleEmiter death (On Emiter update because need to resize before Particle update)
	if (!newPositions.empty())
	{
		for (std::list<math::float3>::const_iterator iterator = newPositions.begin(); iterator != newPositions.end(); ++iterator)
		{
			CreateParticles(rateOverTime, normalShapeType, *iterator);
		}

		newPositions.clear();
	}
}

void ComponentEmitter::ClearEmitter()
{
	for (std::list<Particle*>::iterator iterator = particles.begin(); iterator != particles.end(); ++iterator)
	{
		(*iterator)->active = false;
		(*iterator)->owner = nullptr;
	}

	App->particle->activeParticles -= particles.size();

	particles.clear();
}

void ComponentEmitter::SoftClearEmitter()
{
	App->particle->activeParticles -= particles.size();

	particles.clear();
}


void ComponentEmitter::CreateParticles(int particlesToCreate, ShapeType shapeType, const math::float3& pos)
{
	if (particlesToCreate == 0)
		++particlesToCreate;

	for (int i = 0; i < particlesToCreate; ++i)
	{
		int particleId = 0;
		if (App->particle->GetParticle(particleId))
		{
			math::float3 spawnPos = pos;
			spawnPos += RandPos(shapeType);

			App->particle->allParticles[particleId].SetActive(spawnPos, startValues, textureColumns, textureRows);

			App->particle->allParticles[particleId].owner = this;
			particles.push_back(&App->particle->allParticles[particleId]);
		}
		else
			break;
	}
}

math::float3 ComponentEmitter::RandPos(ShapeType shapeType)
{
	math::float3 spawn = math::float3::zero;

	switch (shapeType)
	{
	case ShapeType_BOX:
		spawn = boxCreation.RandomPointInside(App->randomMathLCG);
		startValues.particleDirection = (math::float3::unitY * parent->transform->rotation.ToFloat3x3()).Normalized();
		break;

	case ShapeType_SPHERE:
		spawn = sphereCreation.RandomPointInside(App->randomMathLCG);
		startValues.particleDirection = spawn.Normalized();
		break;

	case ShapeType_SPHERE_CENTER:
		startValues.particleDirection = sphereCreation.RandomPointInside(App->randomMathLCG).Normalized();
		break;

	case ShapeType_SPHERE_BORDER:
		spawn = sphereCreation.RandomPointOnSurface(App->randomMathLCG);
		startValues.particleDirection = spawn.Normalized();
		break;

	case ShapeType_CONE:
	{
		float angle = 0.0f;
		float centerDist = 0.0f;

		angle = (2 * PI) * (float)App->GenerateRandomNumber() / MAXUINT;
		centerDist = (float)App->GenerateRandomNumber() / MAXUINT;

		circleCreation.pos = (math::float3::unitY * parent->transform->rotation.ToFloat3x3()).Normalized();
		circleCreation.normal = -circleCreation.pos;
		startValues.particleDirection = (circleCreation.GetPoint(angle, centerDist)).Normalized();
		break;
	}
	default:
		break;
	}

	math::float3 global = math::float3::zero;
	if (parent)
		global = parent->transform->position;

	return spawn + global;
}

void ComponentEmitter::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Particle System");
	ImGui::Spacing();

	ParticleValues();

	ParticleShape();

	ParticleColor();

	ParticleBurst();

	ParticleAABB();

	ParticleTexture();

	ParticleSubEmitter();
#endif
}

void ComponentEmitter::ParticleValues()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Values", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ShowHelpMarker("Active checkBox if you want a random number");

		if (ImGui::Checkbox("##Speed", &checkSpeed))
			EqualsMinMaxValues(startValues.speed);
		ShowFloatValue(startValues.speed, checkSpeed, "Speed", 0.25f, 0.25f, 20.0f);

		if (ImGui::Checkbox("##Acceleration", &checkAcceleration))
			EqualsMinMaxValues(startValues.acceleration);
		ShowFloatValue(startValues.acceleration, checkAcceleration, "Acceleration", 0.25f, -5.0f, 5.0f);

		if (ImGui::Checkbox("##Rotation", &checkRotation))
			EqualsMinMaxValues(startValues.rotation);
		ShowFloatValue(startValues.rotation, checkRotation, "Initial Rotation", 0.25f, -360.0f, 360.0f);

		if (ImGui::Checkbox("##AngularVelocity", &checkAngularVelocity))
			EqualsMinMaxValues(startValues.angularVelocity);
		ShowFloatValue(startValues.angularVelocity, checkAngularVelocity, "Angular Velocity", 0.25f, -45.0f, 45.0f);

		if (ImGui::Checkbox("##AngularAcceleration", &checkAngularAcceleration))
			EqualsMinMaxValues(startValues.angularAcceleration);
		ShowFloatValue(startValues.angularAcceleration, checkAngularAcceleration, "Angular Acceleration", 0.25f, -45.0f, 45.0f);

		if (ImGui::Checkbox("##Lifetime", &checkLife))
			EqualsMinMaxValues(startValues.life);
		ShowFloatValue(startValues.life, checkLife, "Lifetime", 0.5f, 1.0f, 20.0f);

		if (ImGui::Checkbox("##Size", &checkSize))
			EqualsMinMaxValues(startValues.size);
		ShowFloatValue(startValues.size, checkSize, "Size", 0.1f, 0.1f, 5.0f);
		
		if (ImGui::Checkbox("##SizeOverTime", &checkSizeOverTime))
			EqualsMinMaxValues(startValues.sizeOverTime);
		ShowFloatValue(startValues.sizeOverTime, checkSizeOverTime, "SizeOverTime", 0.25f, -1.0f, 1.0f);

		ImGui::PushItemWidth(100.0f);
		ImGui::DragInt("Emition", &rateOverTime, 1.0f, 0.0f, 300.0f, "%.2f");

		ImGui::Separator();
		if (ImGui::Checkbox("Loop", &loop))
			loopTimer.Start();
		ImGui::DragFloat("Duration", &duration, 0.5f, 0.5f, 20.0f, "%.2f");
	}
#endif
}

void ComponentEmitter::ParticleShape()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Shape"))
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Change Shape"))
		{
			if (ImGui::MenuItem("Box"))
				normalShapeType = ShapeType_BOX;
			else if (ImGui::MenuItem("Sphere"))
				normalShapeType = ShapeType_SPHERE;
			else if (ImGui::MenuItem("Cone"))
				normalShapeType = ShapeType_CONE;
			ImGui::End();
		}


		math::float3 pos;
		switch (normalShapeType)
		{
		case ShapeType_BOX:
			ImGui::Text("Box");
			pos = boxCreation.Size();
			ImGui::DragFloat3("Box Size", &pos.x, 0.1f, 0.1f, 20.0f, "%.2f");

			boxCreation.SetFromCenterAndSize(boxCreation.CenterPoint(), pos);

			break;
		case ShapeType_SPHERE:
		case ShapeType_SPHERE_BORDER:
		case ShapeType_SPHERE_CENTER:
			ImGui::Text("Sphere");

			ImGui::Text("Particle emision from:");

			if (ImGui::RadioButton("Random", normalShapeType == ShapeType_SPHERE))
				normalShapeType = ShapeType_SPHERE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Center", normalShapeType == ShapeType_SPHERE_CENTER))
				normalShapeType = ShapeType_SPHERE_CENTER;
			ImGui::SameLine();
			if (ImGui::RadioButton("Border", normalShapeType == ShapeType_SPHERE_BORDER))
				normalShapeType = ShapeType_SPHERE_BORDER;

			ImGui::DragFloat("Sphere Size", &sphereCreation.r, 0.25f, 1.0f, 20.0f, "%.2f");

			break;
		case ShapeType_CONE:
			ImGui::Text("Cone");
			ImGui::DragFloat("Sphere Size", &circleCreation.r, 0.25f, 0.25f, 20.0f, "%.2f");

			break;
		default:
			break;
		}
	}
#endif
}

void ComponentEmitter::ParticleColor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Color"))

	{
		ImGui::Text("Particle Color");
		ImGui::SameLine();
		ImGui::ShowHelpMarker("Click color square for change it");
		std::vector<ColorTime> deleteColor;
		std::list<ColorTime>::iterator iter = startValues.color.begin();
		uint posList = 0u;
		while (iter != startValues.color.end())
		{
			//TODO: they must be able to change position
			if ((iter) == startValues.color.begin())
			{//Cant delete 1st color
				ImGui::PushItemWidth(150.0f);
				if (!EditColor(*iter))
					break;
				iter++;
			}
			else
			{
				if (!EditColor(*iter, posList))
					startValues.color.erase(iter++);
				else
					iter++;
			}
			++posList;
		}
		ImGui::Separator();
		ImGui::Checkbox("Color time", &startValues.timeColor);
		if (startValues.timeColor)
		{

			ImGui::DragInt("Position", &nextPos, 1.0f, 1, 100);
			ImGui::ColorPicker4("", &nextColor.x, ImGuiColorEditFlags_AlphaBar);
			if (ImGui::Button("Add Color", ImVec2(125, 25)))
			{
				ColorTime colorTime;
				colorTime.color = nextColor;
				colorTime.position = (float)nextPos / 100;
				colorTime.name = std::to_string((int)nextPos) + "%";
				startValues.color.push_back(colorTime);
				startValues.color.sort();
			}
		}
	}
#endif
}

void ComponentEmitter::ParticleBurst()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Particle Burst"))
	{
		ImGui::Checkbox("Burst", &burst);
		if (ImGui::BeginMenu(burstTypeName.data()))
		{
			if (ImGui::MenuItem("Box"))
			{
				burstType = ShapeType_BOX;
				burstTypeName = "Box Burst";
			}
			else if (ImGui::MenuItem("Sphere"))
			{
				burstType = ShapeType_SPHERE_CENTER;
				burstTypeName = "Sphere Burst";
			}
			else if (ImGui::MenuItem("Cone"))
			{
				burstType = ShapeType_CONE;
				burstTypeName = "Cone Burst";
			}
			ImGui::End();
		}
		ImGui::PushItemWidth(100.0f);
		ImGui::DragInt("Min particles", &minPart, 1.0f, 0, 100);
		if (minPart > maxPart)
			maxPart = minPart;
		ImGui::DragInt("Max Particles", &maxPart, 1.0f, 0, 100);
		if (maxPart < minPart)
			minPart = maxPart;
		ImGui::DragFloat("Repeat Time", &repeatTime, 0.5f, 0.0f, 0.0f, "%.1f");

		ImGui::Separator();
	}
#endif
}

void ComponentEmitter::ParticleAABB()
{
#ifndef GAMEMODE

	if (ImGui::CollapsingHeader("Particle BoundingBox"))
	{
		ImGui::Checkbox("Bounding Box", &drawAABB);
		if (drawAABB)
		{
			math::float3 size = parent->boundingBox.Size();
			if (ImGui::DragFloat3("Dimensions", &size.x, 1.0f, 0.0f, 0.0f, "%.0f"))
				parent->boundingBox.SetFromCenterAndSize(posDifAABB, size);

			if (ImGui::DragFloat3("Pos", &posDifAABB.x, 1.0f, 0.0f, 0.0f, "%.0f"))
				parent->boundingBox.SetFromCenterAndSize(posDifAABB, size);
		}
	}
#endif
}

void ComponentEmitter::ParticleTexture()
{
	if (ImGui::CollapsingHeader("Particle Texture", ImGuiTreeNodeFlags_FramePadding))
	{

		if (ImGui::Checkbox("Animated sprite", &isParticleAnimated))
		{
			if (!isParticleAnimated)
			{
				textureRows = 1;
				textureColumns = 1;
				dieOnAnimation = false;
				SetNewAnimation();
			}
			else
				SetNewAnimation();

			startValues.isAnimated = isParticleAnimated;
		}
		if (isParticleAnimated)
		{
			ImGui::DragFloat("Animation Speed", &animationSpeed, 0.001f, 0.0f, 5.0f, "%.3f");
			ImGui::DragInt("Rows", &textureRows, 1, 1, 10);
			ImGui::DragInt("Columns", &textureColumns, 1, 1, 10);

			ImGui::Checkbox("Kill particle with animation", &dieOnAnimation);
			if (dieOnAnimation)
			{
				checkLife = false;
				startValues.life.x = animationSpeed * textureColumns * textureRows;
			}
			if (ImGui::Button("Instant Animation", ImVec2(150.0f, 25.0f)))
			{
				SetNewAnimation();
			}
		}
		ImGui::Separator();
	}
}

void ComponentEmitter::SetNewAnimation()
{
	for (std::list<Particle*>::iterator iterator = particles.begin(); iterator != particles.end(); ++iterator)
	{
		(*iterator)->ChangeAnim(textureRows, textureColumns, isParticleAnimated);
	}
}

void ComponentEmitter::ParticleSubEmitter()
{
#ifndef GAMEMODE
	if (ImGui::Checkbox("SubEmitter", &startValues.subEmitterActive))
	{
		if (startValues.subEmitterActive)
		{
			if (subEmitter)
				subEmitter->ToggleIsActive();
			else
			{
				subEmitter = App->GOs->CreateGameObject("SubEmition",parent);
				subEmitter->AddComponent(EmitterComponent);
				((ComponentEmitter*)subEmitter->GetComponent(EmitterComponent))->isSubEmitter = true;
				subEmitter->boundingBox.SetFromCenterAndSize(subEmitter->transform->position, math::float3::one);
				App->scene->quadtree.Insert(subEmitter);
			}
		}
		else
			subEmitter->ToggleIsActive();
	}
	ImGui::Separator();
#endif
}

void ComponentEmitter::ShowFloatValue(math::float2& value, bool checkBox, const char* name, float v_speed, float v_min, float v_max)
{
#ifndef GAMEMODE
	ImGui::SameLine();
	if (checkBox)
	{
		ImGui::PushItemWidth(42.0f);
		std::string str = "##";
		str.append(name);
		str.append("min");
		if (ImGui::DragFloat(str.data(), &value.x, v_speed, v_min, v_max, "%.2f"))
			CheckMinMax(value);
		ImGui::SameLine();
		if (ImGui::DragFloat(name, &value.y, v_speed, v_min, v_max, "%.2f"))
			CheckMinMax(value);
	}
	else
	{
		ImGui::PushItemWidth(100.0f);
		if (ImGui::DragFloat(name, &value.x, v_speed, v_min, v_max, "%.2f"))
			value.y = value.x;
	}
	ImGui::PopItemWidth();
#endif
}

void ComponentEmitter::EqualsMinMaxValues(math::float2 & value)
{
#ifndef GAMEMODE
	if(value[1] != value[0])
		value[1] = value[0];
#endif
}

void ComponentEmitter::CheckMinMax(math::float2& value)
{
	if (value.x > value.y)
		value.y = value.x;
}

bool ComponentEmitter::EditColor(ColorTime &colorTime, uint pos)
{
	bool ret = true;

#ifndef GAMEMODE



	ImVec4 color = EqualsFloat4(colorTime.color);
	if (ImGui::ColorButton(colorTime.name.data(), color, ImGuiColorEditFlags_None, ImVec2(100, 20)))
		colorTime.changingColor = !colorTime.changingColor;

	if (!colorTime.changingColor)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(colorTime.name.data());
		if (pos > 0)
		{
			std::string colorStr = "Remove Color ";
			colorStr.append(std::to_string(pos));
			ImGui::SameLine();
			if (ImGui::Button(colorStr.data(), ImVec2(125, 25)))
				ret = false;
		}
		else if (!startValues.timeColor)
			ret = false;
	}
	else
		ImGui::ColorEdit4(colorTime.name.data(), &colorTime.color.x, ImGuiColorEditFlags_AlphaBar);

#endif
	return ret;
}

#ifndef GAMEMODE
ImVec4 ComponentEmitter::EqualsFloat4(const math::float4 float4D)
{
	ImVec4 vec;
	vec.x = float4D.x;
	vec.y = float4D.y;
	vec.z = float4D.z;
	vec.w = float4D.w;
	return vec;
}
#endif

uint ComponentEmitter::GetInternalSerializationBytes()
{
	//		Value Checkers +	StartValues
	return sizeof(bool)*8 + sizeof(StartValues) + sizeof(rateOverTime) + sizeof(duration)
		 + sizeof(drawAABB) + sizeof(isSubEmitter) + sizeof(uint)//UUID Subemiter
		 + sizeof(dieOnAnimation) + sizeof(isParticleAnimated) + sizeof(animationSpeed)
		 + sizeof(textureColumns) + sizeof(textureRows) + sizeof(normalShapeType)
		 + sizeof(boxCreation) + sizeof(float)*2 //Circle and Sphere rad
		 + sizeof(gravity) + sizeof(posDifAABB) + sizeof(loop) + sizeof(burst)
		 + sizeof(minPart) + sizeof(maxPart) + sizeof(repeatTime);
}

void ComponentEmitter::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(StartValues);
	memcpy(cursor, &startValues, bytes);
	cursor += bytes;

	//Checkers
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkLife, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkSpeed, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkAcceleration, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkSizeOverTime, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkSize, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkRotation, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkAngularAcceleration, bytes);
	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &checkAngularVelocity, bytes);

	cursor += bytes;
	size_t bytes = sizeof(int);
	memcpy(cursor, &rateOverTime, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &duration, bytes);
	cursor += bytes;

	size_t bytes = sizeof(bool);
	memcpy(cursor, &drawAABB, bytes);

	cursor += bytes;
	size_t bytes = sizeof(bool);
	memcpy(cursor, &isSubEmitter, bytes);


	uint uuid = 0u;
	if (subEmitter)
		 uuid = subEmitter->GetUUID();

	cursor += bytes;
	size_t bytes = sizeof(uint);
	memcpy(cursor, &uuid, bytes);
	cursor += bytes;


	size_t bytes = sizeof(bool);
	memcpy(cursor, &dieOnAnimation, bytes);
	cursor += bytes;	

	size_t bytes = sizeof(bool);
	memcpy(cursor, &isParticleAnimated, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &animationSpeed, bytes);
	cursor += bytes;

	size_t bytes = sizeof(int);
	memcpy(cursor, &textureColumns, bytes);
	cursor += bytes;

	size_t bytes = sizeof(int);
	memcpy(cursor, &textureRows, bytes);
	cursor += bytes;

	size_t bytes = sizeof(ShapeType);
	memcpy(cursor, &normalShapeType, bytes);
	cursor += bytes;

	size_t bytes = sizeof(math::AABB);
	memcpy(cursor, &boxCreation, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &circleCreation.r, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &sphereCreation.r, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &gravity, bytes);
	cursor += bytes;

	size_t bytes = sizeof(math::float3);
	memcpy(cursor, &posDifAABB, bytes);
	cursor += bytes;

	size_t bytes = sizeof(bool);
	memcpy(cursor, &loop, bytes);
	cursor += bytes;

	size_t bytes = sizeof(bool);
	memcpy(cursor, &burst, bytes);
	cursor += bytes;

	size_t bytes = sizeof(int);
	memcpy(cursor, &minPart, bytes);
	cursor += bytes;

	size_t bytes = sizeof(int);
	memcpy(cursor, &maxPart, bytes);
	cursor += bytes;

	size_t bytes = sizeof(float);
	memcpy(cursor, &repeatTime, bytes);
	cursor += bytes;
}

void ComponentEmitter::OnInternalLoad(char *& cursor)
{
}

int ComponentEmitter::GetEmition() const
{
	return rateOverTime;
}
// todo event system to delete texture