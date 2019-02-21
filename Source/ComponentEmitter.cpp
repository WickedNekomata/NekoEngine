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

	// TODO
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
	texture = componentEmitter.texture;

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
		textureRows = componentEmitter.textureRows;
		textureColumns = componentEmitter.textureColumns;
		animationSpeed = componentEmitter.animationSpeed;
	}
	dieOnAnimation = componentEmitter.dieOnAnimation;

	drawAABB = componentEmitter.drawAABB;

	isSubEmitter = componentEmitter.isSubEmitter;
	subEmitter = componentEmitter.subEmitter;
	subEmitterUUID = componentEmitter.subEmitterUUID;

	rateOverTime = componentEmitter.rateOverTime;

	if (parent)
	//	parent->boundingBox = math::AABB::FromCenterAndSize(posDifAABB, sizeOBB);
	App->scene->quadtree.Insert(parent);

	App->particle->emitters.push_back(this);

	// TODO
	//material = (ComponentMaterial*)parent->AddComponent(ComponentTypes::MaterialComponent);
}


ComponentEmitter::~ComponentEmitter()
{
	if (App)
	{
		App->timeManager->RemoveGameTimer(&timer);
		App->timeManager->RemoveGameTimer(&burstTime);
		App->timeManager->RemoveGameTimer(&loopTimer);
		App->timeManager->RemoveGameTimer(&timeSimulating);

		App->particle->RemoveEmitter(this);
	}			
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

			App->particle->allParticles[particleId].SetActive(spawnPos, startValues, &texture, /*&particleAnimation.textureIDs,*/ animationSpeed);

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

	//ParticleTexture();

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
/*
void ComponentEmitter::ParticleTexture()
{
	if (ImGui::CollapsingHeader("Particle Texture", ImGuiTreeNodeFlags_FramePadding))
	{
		if (texture)
		{
			std::string name = texture->file;
			name = name.substr(name.find_last_of("\\") + 1);

			ImGui::Text("Loaded texture '%s'", name.data());
			//ImGui::Text("Texture used %i times", texture->usage);

			ImGui::Image((void*)(intptr_t)texture->id, ImVec2(256.0f, 256.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			if (ImGui::BeginMenu("Change Texture"))
			{
				std::vector<Resource*> resource;
				App->res->GetResources(resource, ResourceType::TextureResource);

				for (std::vector<Resource*>::iterator iterator = resource.begin(); iterator != resource.end(); ++iterator)
				{
					std::string textName;
					textName.append((*iterator)->GetName());
					if (ImGui::MenuItem(textName.data()))
					{
						App->res->Remove(texture);
						texture = nullptr;

						texture = ((ResourceTexture*)(*iterator));
						texture->usage++;
					}
				}
				ImGui::End();
			}
			if (ImGui::Button("Remove Texture", ImVec2(125, 25)))
			{
				App->res->Remove(texture);
				texture = nullptr;
			}

		}
		else
		{
			ImGui::Text("No texture loaded");
			if (ImGui::BeginMenu("Add new Texture"))
			{
				std::vector<Resource*> resource;
				App->res->GetResources(resource, ResourceType::TextureResource);

				for (std::vector<Resource*>::iterator iterator = resource.begin(); iterator != resource.end(); ++iterator)
				{
					std::string textName;
					textName.append((*iterator)->GetName());
					if (ImGui::MenuItem(textName.data()))
					{
						texture = ((ResourceTexture*)(*iterator));
						texture->usage++;
					}
				}
				ImGui::End();
			}
		}

		ImGui::Separator();
		if (ImGui::Checkbox("Animated sprite", &isParticleAnimated))
		{
			if (!isParticleAnimated)
			{
				SetNewAnimation(1, 1);
				dieOnAnimation = false;
			}
			else
				SetNewAnimation(textureRows, textureColumns);
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
				startValues.life.x = animationSpeed * particleAnimation.columns * particleAnimation.rows;
			}

			if (ImGui::Button("Calc Animation", ImVec2(150.0f, 25.0f)))
			{
				SetNewAnimation(textureRows, textureColumns);
			}
		}
		ImGui::Separator();
	}
}
*/
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
/*
void ComponentEmitter::SetNewAnimation(int row, int col)
{
	particleAnimation = App->res->LoadTextureUV(row, col);
	for (std::list<Particle*>::iterator iterator = particles.begin(); iterator != particles.end(); ++iterator)
	{
		(*iterator)->currentFrame = 0;
	}
}
*/
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
	return 0;
}

void ComponentEmitter::OnInternalSave(JSON_Object* parent)
{
	json_object_set_number(parent, "Type", this->componentType);

	json_object_set_number(parent, "UUID", this->parent->GetUUID());

	//json_object_set_number(parent, "Time Created", GetTime());

	json_object_set_boolean(parent, "checkLife", checkLife);
	json_object_set_boolean(parent, "checkSpeed", checkSpeed);
	json_object_set_boolean(parent, "checkAcceleration", checkAcceleration);
	json_object_set_boolean(parent, "checkSize", checkSize);
	json_object_set_boolean(parent, "checkSizeOverTime", checkSizeOverTime);
	json_object_set_boolean(parent, "checkRotation", checkRotation);
	json_object_set_boolean(parent, "checkAngularAcceleration", checkAngularAcceleration);
	json_object_set_boolean(parent, "checkAngularVelocity", checkAngularVelocity);

	json_object_set_number(parent, "lifeMin", startValues.life.x);
	json_object_set_number(parent, "lifeMax", startValues.life.y);

	json_object_set_number(parent, "speedMin", startValues.speed.x);
	json_object_set_number(parent, "speedMax", startValues.speed.y);

	json_object_set_number(parent, "accelerationMin", startValues.acceleration.x);
	json_object_set_number(parent, "accelerationMax", startValues.acceleration.y);

	json_object_set_number(parent, "sizeMin", startValues.size.x);
	json_object_set_number(parent, "sizeMax", startValues.size.y);

	json_object_set_number(parent, "sizeOverTimeMin", startValues.sizeOverTime.x);
	json_object_set_number(parent, "sizeOverTimeMax", startValues.sizeOverTime.y);

	json_object_set_number(parent, "rotationMin", startValues.rotation.x);
	json_object_set_number(parent, "rotationMax", startValues.rotation.y);

	json_object_set_number(parent, "angularAccelerationMin", startValues.angularAcceleration.x);
	json_object_set_number(parent, "angularAccelerationMax", startValues.angularAcceleration.y);

	json_object_set_number(parent, "angularVelocityMin", startValues.angularVelocity.x);
	json_object_set_number(parent, "angularVelocityMax", startValues.angularVelocity.y);


	json_object_set_boolean(parent, "subEmitterActive", startValues.subEmitterActive);


	json_object_set_number(parent, "rateOverTime", rateOverTime);

	JSON_Value* colorValue = json_value_init_array();
	JSON_Array* color = json_value_get_array(colorValue);

	for (std::list<ColorTime>::const_iterator iterator = startValues.color.begin(); iterator != startValues.color.end(); ++iterator)
	{
		JSON_Value* newColor = json_value_init_object();
		JSON_Object* objCol = json_value_get_object(newColor);

		json_object_set_number(objCol, "colorX", (*iterator).color.x);
		json_object_set_number(objCol, "colorY", (*iterator).color.y);
		json_object_set_number(objCol, "colorZ", (*iterator).color.z);
		json_object_set_number(objCol, "colorW", (*iterator).color.w);

		json_object_set_number(objCol, "position", (*iterator).position);
		json_object_set_string(objCol, "name", (*iterator).name.data());


		json_array_append_value(color, newColor);
	}
		json_object_set_value(parent, "Colors", colorValue);

	// TODO: save colors
	json_object_set_number(parent, "timeColor", startValues.timeColor);

	json_object_set_number(parent, "particleDirectionX", startValues.particleDirection.x);
	json_object_set_number(parent, "particleDirectionY", startValues.particleDirection.y);
	json_object_set_number(parent, "particleDirectionZ", startValues.particleDirection.z);

	json_object_set_number(parent, "duration", duration);

	json_object_set_number(parent, "loop", loop);

	json_object_set_number(parent, "burst", burst);
	json_object_set_number(parent, "minPart", minPart);
	json_object_set_number(parent, "maxPart", maxPart);
	json_object_set_number(parent, "repeatTime", repeatTime);

	json_object_set_number(parent, "posDifAABBX",posDifAABB.x);
	json_object_set_number(parent, "posDifAABBY",posDifAABB.y);
	json_object_set_number(parent, "posDifAABBZ",posDifAABB.z);

	json_object_set_number(parent, "gravity", gravity);

	json_object_set_number(parent, "boxCreationMinX", boxCreation.minPoint.x);
	json_object_set_number(parent, "boxCreationMinY", boxCreation.minPoint.y);
	json_object_set_number(parent, "boxCreationMinZ", boxCreation.minPoint.z);

	json_object_set_number(parent, "boxCreationMaxX", boxCreation.maxPoint.x);
	json_object_set_number(parent, "boxCreationMaxY", boxCreation.maxPoint.y);
	json_object_set_number(parent, "boxCreationMaxZ", boxCreation.maxPoint.z);
	
	SaveNumberArray(parent, "boxCreationMin", boxCreation.minPoint.ptr(), 3);
	SaveNumberArray(parent, "boxCreationMax", boxCreation.maxPoint.ptr(), 3);

	json_object_set_number(parent, "SphereCreationRad", sphereCreation.r);

	json_object_set_number(parent, "circleCreationRad", circleCreation.r);

	json_object_set_number(parent, "shapeType", normalShapeType);

	/*if (texture)
	json_object_set_string(parent, "texture", texture->file.data());
	else*/
	json_object_set_string(parent, "texture", "noTexture");

	json_object_set_number(parent, "textureRows", textureRows);
	json_object_set_number(parent, "textureColumns", textureColumns);
	json_object_set_number(parent, "animationSpeed", animationSpeed);

	json_object_set_boolean(parent, "isParticleAnimated", isParticleAnimated);
	json_object_set_boolean(parent, "dieOnAnimation", dieOnAnimation);

	json_object_set_boolean(parent, "drawAABB", drawAABB);
	
	json_object_set_boolean(parent, "isSubEmitter", isSubEmitter);
	if(subEmitter)
	json_object_set_number(parent, "SubEmitter", subEmitter->GetUUID());


	if (this->parent && this->parent->transform)
	{
		math::float3 bb = this->parent->boundingBox.Size();

		json_object_set_number(parent, "originalBoundingBoxSizeX", bb.x);
		json_object_set_number(parent, "originalBoundingBoxSizeY", bb.y);
		json_object_set_number(parent, "originalBoundingBoxSizeZ", bb.z);
	}
}

void ComponentEmitter::OnLoad(JSON_Object* info)
{

	duration = json_object_get_number(info, "duration");

	loop = json_object_get_number(info, "loop");

	burst = json_object_get_number(info, "burst");
	minPart = json_object_get_number(info, "minPart");
	maxPart = json_object_get_number(info, "maxPart");
	repeatTime = json_object_get_number(info, "repeatTime");

	isSubEmitter = json_object_get_boolean(info, "isSubEmitter");

	// posDifAABB
	math::float3 posDifAABB = math::float3(
		json_object_get_number(info, "posDifAABBX"),
		json_object_get_number(info, "posDifAABBY"),
		json_object_get_number(info, "posDifAABBZ"));
	posDifAABB = posDifAABB;

	gravity = json_object_get_number(info, "gravity");

	// boxCreation
	math::float3 boxMin = math::float3(
		json_object_get_number(info, "boxCreationMinX"),
		json_object_get_number(info, "boxCreationMinY"),
		json_object_get_number(info, "boxCreationMinZ"));

	math::float3 boxMax = math::float3(
		json_object_get_number(info, "boxCreationMaxX"),
		json_object_get_number(info, "boxCreationMaxY"),
		json_object_get_number(info, "boxCreationMaxZ"));

	boxCreation = math::AABB(boxMin, boxMax);

	sphereCreation.r = json_object_get_number(info, "SphereCreationRad");

	circleCreation.r = json_object_get_number(info, "circleCreationRad");

	normalShapeType = (ShapeType)(int)json_object_get_number(info, "shapeType");
	//texture = App->resources->LoadTexture(json_object_get_string(info, "texture"));

	isParticleAnimated = json_object_get_boolean(info, "isParticleAnimated");
	dieOnAnimation = json_object_get_boolean(info, "dieOnAnimation");
	textureColumns = json_object_get_number(info, "textureColumns");
	textureRows = json_object_get_number(info, "textureRows");
	animationSpeed = json_object_get_number(info, "animationSpeed");

	//AABB Colision
	drawAABB = json_object_get_boolean(info, "drawAABB");

	checkLife = json_object_get_boolean(info, "checkLife");
	checkSpeed = json_object_get_boolean(info, "checkSpeed");
	checkAcceleration = json_object_get_boolean(info, "checkAcceleration");
	checkSize = json_object_get_boolean(info, "checkSize");
	checkSizeOverTime = json_object_get_boolean(info, "checkSizeOverTime");
	checkRotation = json_object_get_boolean(info, "checkRotation");
	checkAngularAcceleration = json_object_get_boolean(info, "checkAngularAcceleration");
	checkAngularVelocity = json_object_get_boolean(info, "checkAngularVelocity");

	subEmitterUUID = json_object_get_number(info, "SubEmitter");

	rateOverTime = json_object_get_number(info, "rateOverTime");

	startValues.subEmitterActive = json_object_get_boolean(info, "subEmitterActive");

	startValues.life.x = json_object_get_number(info, "lifeMin");
	startValues.life.y = json_object_get_number(info, "lifeMax");

	startValues.speed.x = json_object_get_number(info, "speedMin");
	startValues.speed.y = json_object_get_number(info, "speedMax");

	startValues.acceleration.x = json_object_get_number(info, "accelerationMin");
	startValues.acceleration.y = json_object_get_number(info, "accelerationMax");

	startValues.size.x = json_object_get_number(info, "sizeMin");
	startValues.size.y = json_object_get_number(info, "sizeMax");

	startValues.rotation.x = json_object_get_number(info, "rotationMin");
	startValues.rotation.y = json_object_get_number(info, "rotationMax");

	startValues.angularVelocity.x = json_object_get_number(info, "angularVelocityMin");
	startValues.angularVelocity.y = json_object_get_number(info, "angularVelocityMax");

	startValues.angularAcceleration.x = json_object_get_number(info, "angularAccelerationMin");
	startValues.angularAcceleration.y = json_object_get_number(info, "angularAccelerationMax");

	startValues.sizeOverTime.x = json_object_get_number(info, "sizeOverTimeMin");
	startValues.sizeOverTime.y = json_object_get_number(info, "sizeOverTimeMax");

	JSON_Array* colorArray = json_object_get_array(info, "Colors");

	int numColors = json_array_get_count(colorArray);

	startValues.color.clear();
	startValues.color.resize(numColors);
	int i = 0;
	for (std::list<ColorTime>::iterator iterator = startValues.color.begin(); iterator != startValues.color.end(); ++iterator, ++i)
	{
		JSON_Object* currCol = json_array_get_object(colorArray, i);

		(*iterator).color = math::float4(
			json_object_get_number(currCol, "colorX"),
			json_object_get_number(currCol, "colorY"),
			json_object_get_number(currCol, "colorZ"),
			json_object_get_number(currCol, "colorW")
		);

		(*iterator).position = json_object_get_number(currCol, "position");

		(*iterator).name = json_object_get_string(currCol, "name");
	}


	startValues.timeColor = json_object_get_number(info, "timeColor");

	startValues.particleDirection.x = json_object_get_number(info, "particleDirectionX");
	startValues.particleDirection.y = json_object_get_number(info, "particleDirectionY");
	startValues.particleDirection.x = json_object_get_number(info, "particleDirectionZ");
}

int ComponentEmitter::GetEmition() const
{
	return rateOverTime;
}

// todo event system to delete texture