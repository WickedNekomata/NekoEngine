#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ResourcePrefab.h"

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stl.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>

#include <mono/metadata/attrdefs.h>


ComponentScript::~ComponentScript()
{
	if(scriptRes)
		scriptRes->deReferenced();

	if (handleID != 0)
	{
		mono_gchandle_free(handleID);
		handleID = 0;
	}
}

void ComponentScript::Awake()
{
	if (scriptRes && scriptRes->awakeMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->awakeMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::Start()
{
	if (scriptRes && scriptRes->startMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->startMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::PreUpdate()
{
	if (scriptRes && scriptRes->preUpdateMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->preUpdateMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::Update()
{
	if (scriptRes && scriptRes->updateMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->updateMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::PostUpdate()
{
	if (scriptRes && scriptRes->postUpdateMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->postUpdateMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnEnableMethod()
{
	if (scriptRes && scriptRes->enableMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->enableMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::OnDisableMethod()
{
	if (scriptRes && scriptRes->disableMethod)
	{
		MonoObject* exc = nullptr;
		mono_runtime_invoke(scriptRes->disableMethod, classInstance, NULL, &exc);
		if (exc)
		{
			Event event;
			event.type = EventType::PAUSE;
			App->SendEvent(event);
			App->time->paused = true;

			MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
			char* toLogMessage = mono_string_to_utf8(exceptionMessage);
			Debug.LogError(toLogMessage);
			mono_free(toLogMessage);
		}
	}
}

void ComponentScript::OnStop()
{
	if (scriptRes && scriptRes->stopMethod)
	{
		MonoObject* exc = nullptr;
		if (isActive() && gameObject->areParentsActives())
		{
			mono_runtime_invoke(scriptRes->stopMethod, classInstance, NULL, &exc);
			if (exc)
			{
				Event event;
				event.type = EventType::PAUSE;
				App->SendEvent(event);
				App->time->paused = true;

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				Debug.LogError(toLogMessage);
				mono_free(toLogMessage);
			}
		}
	}
}

void ComponentScript::onEnable()
{
	App->scripting->UpdateMonoObjects();

	OnEnableMethod();

	App->scripting->UpdateGameObjects();
}

void ComponentScript::onDisable()
{
	App->scripting->UpdateMonoObjects();

	OnDisableMethod();

	App->scripting->UpdateGameObjects();
}

void ComponentScript::OnInspector()
{
	if (ImGui::Checkbox(("###ACTIVE_SCRIPT" + std::to_string(UUID)).data(), &this->active))
	{
		if (this->isActive())
		{
			if (IN_GAME)
				this->OnEnableMethod();
		}
		else
		{
			if (IN_GAME)
				this->OnDisableMethod();
		}
	}
	ImGui::SameLine();

	float PosX = ImGui::GetCursorPosX();
	bool opened = ImGui::CollapsingHeader(std::string("##Script" + std::to_string(UUID)).data()); ImGui::SameLine();

	if (ImGui::IsItemClicked(1))
	{
		ImGui::OpenPopup(std::string("##Script" + std::to_string(UUID)).data());
	}

	ImGui::SetNextWindowSize({ 150, 45 });

	ImGuiWindowFlags wflags = 0;
	wflags |= ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;

	if (ImGui::BeginPopup(std::string("##Script" + std::to_string(UUID)).data(), wflags))
	{
		if (ImGui::MenuItem("Delete Component"))
		{
			Event event;
			event.type = EventType::COMPONENT_DESTROYED;
			event.compEvent.component = this;
			App->SendEvent(event);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGuiDragDropFlags flags = 0;
	flags |= ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
	if (ImGui::BeginDragDropSource(flags))
	{
		ImGui::BeginTooltip();
		ImGui::Text(std::string(scriptName + " (Script)").data());
		ImGui::EndTooltip();
		ComponentScript* thisOne = (ComponentScript*)this;
		ImGui::SetDragDropPayload("DraggingComponents", &thisOne, sizeof(ComponentScript));
		ImGui::EndDragDropSource();
	}

	ImGui::SetCursorPosX(PosX + 20);
	ImGui::Text(std::string(scriptName + " (Script)").data());
	if (opened)
	{
		ImGui::NewLine();
		ImGui::Text(".cs File:  "); ImGui::SameLine();

		ImVec2 drawingPos = ImGui::GetCursorScreenPos();
		drawingPos = { drawingPos.x - 10, drawingPos.y };
		ImGui::SetCursorScreenPos(drawingPos);

		uint buttonWidth = 2 * ImGui::GetWindowWidth() / 3;
		ImGui::ButtonEx("##csFile", { (float)buttonWidth, 15 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

		if (ImGui::IsItemHovered())
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(drawingPos, { drawingPos.x + buttonWidth, drawingPos.y + 15 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));
		}

		if (ImGui::IsItemClicked(0))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(drawingPos, { drawingPos.x + buttonWidth, drawingPos.y + 15 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));			
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("\"%s\"\n\nThe .cs file attached to this script component.\nDo not move the script for now!", scriptRes->getFile().data());
			ImGui::EndTooltip();
		}		

		ImGui::SetCursorScreenPos({ drawingPos.x + 7, drawingPos.y });

		//Calculate the text fitting the button rect
		std::string originalText = scriptRes ? scriptRes->getFile().data() : "";
		std::string clampedText;

		ImVec2 textSize = ImGui::CalcTextSize(originalText.data());

		if (textSize.x > buttonWidth - 5)
		{
			uint maxTextLenght = originalText.length() * (buttonWidth - 5) / textSize.x;
			clampedText = originalText.substr(0, maxTextLenght - 5);
			clampedText.append("(...)");
		}
		else
			clampedText = originalText;

		ImGui::Text(clampedText.data());

		ImGui::NewLine();


		if (!classInstance)
		{
			ImGui::TextColored({ .5,0,0,1 }, "SCRIPT WITH ERRORS, CHECK IT");
			return;
		}

		//Script fields
		void* iterator = 0;
		MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

		while (field != nullptr)
		{
			uint32_t flags = mono_field_get_flags(field);
			if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
			{
				//This field is public and not static.
				//Show the field, check the type and adapt the gui to it.
				MonoType* type = mono_field_get_type(field);
				
				std::string typeName = mono_type_full_name(type);

				std::string fieldName = mono_field_get_name(field);

				if (typeName == "bool")
				{
					bool varState; mono_field_get_value(classInstance, field, &varState);
					
					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({cursorPos.x, cursorPos.y + 5});

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y -5 });

					if(ImGui::Checkbox(("##" + fieldName).data(), &varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "single") //this is a float, idk
				{					
					float varState; mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputFloat(("##" + fieldName).data(), &varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "double")
				{
					double varState;
					mono_field_get_value(classInstance, field, &varState);
					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputDouble(("##" + fieldName).data(),&varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "System.Decimal")
				{
					//We cant convert System.Decimal, since we do not have this decimal precision in any C++ type.
				}
				else if (typeName == "sbyte")
				{
					int8_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "byte")
				{
					uint8_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(classInstance, field, &varState);
					}
				}			
				else if (typeName == "int16")
				{
					int16_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "uint16")
				{
					uint16_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					int varState_int = (int)varState;
					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState_int))
					{
						varState = varState_int;
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "int")
				{
					int32_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S32, &varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "uint")
				{
					uint32_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U32, &varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "long")
				{
					int64_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_S64, &varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}
				else if (typeName == "ulong")
				{
					uint64_t varState;
					mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					if (ImGui::InputScalar(("##" + fieldName).data(), ImGuiDataType_U64,&varState))
					{
						mono_field_set_value(classInstance, field, &varState);
					}
				}			
				else if (typeName == "char")
				{
					int temp;
					mono_field_get_value(classInstance, field, &temp);

					char varState = (char)temp;

					std::string stringToModify = std::string(1,varState);
					if (ImGui::InputText(mono_field_get_name(field), &stringToModify))
					{
						MonoString* newString = mono_string_new(App->scripting->domain, stringToModify.data());
						temp = (int)stringToModify[0];
						mono_field_set_value(classInstance, field, &temp);
					}
				}
				else if (typeName == "string")
				{
					MonoString* varState;
					mono_field_get_value(classInstance, field, &varState);

					char* convertedString = mono_string_to_utf8(varState);

					std::string stringToModify = convertedString;
					if (ImGui::InputText(mono_field_get_name(field), &stringToModify))
					{
						MonoString* newString = mono_string_new(App->scripting->domain, stringToModify.data());
						mono_field_set_value(classInstance, field, newString);
					}

					mono_free(convertedString);
				}
				else if (typeName == "FlanEngine.GameObject")
				{
					uint buttonWidth = 0.65 * ImGui::GetWindowWidth();

					float varState; mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					cursorPos = { cursorPos.x, cursorPos.y - 5 };

					ImGui::ButtonEx(("##" + fieldName).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

					//Case 1: Dragging Real GameObjects
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DraggingGOs", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							GameObject* go = *(GameObject**)payload->Data;
							
							if (ImGui::IsMouseReleased(0))
							{
								MonoObject* monoObject = App->scripting->MonoObjectFrom(go);
								mono_field_set_value(classInstance, field, monoObject);
							}
						}
						ImGui::EndDragDropTarget();
					}

					//Case 2: Dragging Prefabs
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DraggingResources", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							Resource* resource = *(Resource**)payload->Data;
							if (resource->getType() == Resource::ResourceType::PREFAB)
							{
								if (ImGui::IsMouseReleased(0))
								{
									ResourcePrefab* prefab = (ResourcePrefab*)resource;

									MonoObject* monoObject = App->scripting->MonoObjectFrom(prefab->GetRoot());
									mono_field_set_value(classInstance, field, monoObject);
								}
							}						
						}
						ImGui::EndDragDropTarget();
					}

					if (ImGui::IsItemClicked(0))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
					}
					else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

						if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
						{
							mono_field_set_value(classInstance, field, NULL);
						}
					}

					//Button text
					MonoObject* monoObject;
					mono_field_get_value(classInstance, field, &monoObject);

					std::string text;

					if (monoObject)
					{
						bool destroyed;
						mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "destroyed"), &destroyed);

						if (!destroyed)
						{
							MonoString* goName;
							mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "name"), &goName);

							char* nameCpp = mono_string_to_utf8(goName);

							GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
							if (gameObject->prefab)
								text = nameCpp + std::string(" (Prefab)");
							else
								text = nameCpp + std::string(" (GameObject)");

							mono_free(nameCpp);
						}
						else
						{
							mono_field_set_value(classInstance, field, NULL);
						}
					}
					else
					{
						text = "NULL (GameObject)";
					}

					ImGui::SetCursorScreenPos({ cursorPos.x+5, cursorPos.y+3});

					ImGui::Text(text.data());	

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });
				}
				else if (typeName == "FlanEngine.Transform")
				{
					uint buttonWidth = 0.65 * ImGui::GetWindowWidth();

					float varState; mono_field_get_value(classInstance, field, &varState);

					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 5 });

					ImGui::Text(fieldName.data()); ImGui::SameLine();

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 5 });

					cursorPos = { cursorPos.x, cursorPos.y - 5 };

					ImGui::ButtonEx(("##" + fieldName).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DraggingGOs", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
						if (payload)
						{
							GameObject* go = *(GameObject**)payload->Data;

							if (ImGui::IsMouseReleased(0))
							{
								MonoObject* monoObject = App->scripting->MonoObjectFrom(go);

								MonoObject* monoTransform; 
								mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "transform"), &monoTransform);

								mono_field_set_value(classInstance, field, monoTransform);
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (ImGui::IsItemClicked(0))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
					}
					else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

						if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
						{
							mono_field_set_value(classInstance, field, NULL);
						}
					}

					//Button text
					MonoObject* monoTransform;
					mono_field_get_value(classInstance, field, &monoTransform);

					MonoObject* monoObject;
					if(monoTransform)
						mono_field_get_value(monoTransform, mono_class_get_field_from_name(mono_object_get_class(monoTransform), "gameObject"), &monoObject);

					std::string text;

					if (monoTransform)
					{
						bool destroyed;
						mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "destroyed"), &destroyed);

						if (!destroyed)
						{
							MonoString* goName;
							mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "name"), &goName);

							char* nameCpp = mono_string_to_utf8(goName);

							text = nameCpp + std::string(" (Transform)");

							mono_free(nameCpp);
						}
						else
						{
							mono_field_set_value(classInstance, field, NULL);
						}
					}
					else
					{
						text = "NULL (Transform)";
					}

					ImGui::SetCursorScreenPos({ cursorPos.x + 5, cursorPos.y + 3 });

					ImGui::Text(text.data());

					cursorPos = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });
				}
			}

			field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
		}
		ImGui::NewLine();
	}
}

uint ComponentScript::bytesToSerializePublicVars() const
{
	uint bytes = sizeof(uint);

	void* iterator = 0;

	if (!classInstance)
		return 0;

	MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			MonoType* type = mono_field_get_type(field);
			std::string typeName = mono_type_full_name(type);
			std::string fieldName = mono_field_get_name(field);

			VarType varType;

			if (typeName == "bool")
			{
				varType = VarType::BOOL;

				uint nameLenght = fieldName.length();	
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(bool));
			}
			else if (typeName == "single")
			{
				varType = VarType::FLOAT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(float));
			}
			else if (typeName == "double")
			{
				varType = VarType::DOUBLE;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(double));
			}
			else if (typeName == "sbyte")
			{
				varType = VarType::INT8;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int8_t));
			}
			else if (typeName == "byte")
			{
				varType = VarType::UINT8;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint8_t));
			}
			else if (typeName == "int16")
			{
				varType = VarType::INT16;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int16_t));
			}
			else if (typeName == "uint16")
			{
				varType = VarType::UINT16;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint16_t));
			}
			else if (typeName == "int")
			{
				varType = VarType::INT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(int));
			}
			else if (typeName == "uint")
			{
				varType = VarType::UINT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint));
			}
			else if (typeName == "long")
			{
				varType = VarType::INT64;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(long));
			}
			else if (typeName == "ulong")
			{
				varType = VarType::UINT64;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint64_t));
			}
			else if (typeName == "char")
			{
				varType = VarType::CHAR;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(char));
			}
			else if (typeName == "string")
			{
				varType = VarType::STRING;

				uint nameLenght = fieldName.length();

				MonoString* varState; mono_field_get_value(classInstance, field, &varState);
				char* cString = mono_string_to_utf8(varState);

				std::string defString(cString);
				uint stringLenght = defString.size();

				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(uint) + stringLenght);

				mono_free(cString);
			}
			else if (typeName == "FlanEngine.GameObject")
			{
				varType = VarType::GAMEOBJECT;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(UID));
			}
			else if (typeName == "FlanEngine.Transform")
			{
				varType = VarType::TRANSFORM;

				uint nameLenght = fieldName.length();
				bytes += (sizeof(varType) + sizeof(uint) + nameLenght + sizeof(UID));
			}
		}
		field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
	}
	return bytes;
}

void ComponentScript::Serialize(char*& cursor) const
{
	uint bytes = sizeof(UID);
	memcpy(cursor, &gameObject->uuid, bytes);
	cursor += bytes;

	memcpy(cursor, &UUID, bytes);
	cursor += bytes;

	UID resUID = scriptRes ? scriptRes->getUUID() : 0;
	memcpy(cursor, &resUID, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &active, bytes);
	cursor += bytes;
}

void ComponentScript::deSerialize(char*& cursor, uint32_t& goUUID)
{
	uint bytes = sizeof(UID);
	memcpy(&goUUID, cursor, bytes);
	cursor += bytes;

	memcpy(&UUID, cursor, bytes);
	cursor += bytes;

	UID resUID;
	memcpy(&resUID, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&active, cursor, bytes);
	cursor += bytes;

	//Reference the ScriptResource
	scriptRes = (ResourceScript*)App->resources->Get(resUID);

	if (scriptRes)
	{
		scriptName = scriptRes->scriptName;
	}		
	else
		Debug.LogError("A ComponentScript lost his ResourceScript reference!");
}

void ComponentScript::SerializePublicVars(char*& cursor) const
{
	uint numVars = 0;

	void* iterator = 0;

	if (!classInstance)
		return;

	MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);
	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			numVars++;
		}
		field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
	}

	uint bytes = sizeof(uint);
	memcpy(cursor, &numVars, bytes);
	cursor += bytes;

	iterator = 0;
	field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);
	while (field != nullptr)
	{
		uint32_t flags = mono_field_get_flags(field);
		if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
		{
			MonoType* type = mono_field_get_type(field);
			std::string typeName = mono_type_full_name(type);
			std::string fieldName = mono_field_get_name(field);

			VarType varType;

			if (typeName == "bool")
			{
				varType = VarType::BOOL;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;
				
				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(bool);
				bool varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "single")
			{
				varType = VarType::FLOAT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(float);
				float varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "double")
			{
				varType = VarType::DOUBLE;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(double);
				double varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "sbyte")
			{
				varType = VarType::INT8;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int8_t);
				int8_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "byte")
			{
				varType = VarType::UINT8;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint8_t);
				uint8_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "int16")
			{
				varType = VarType::INT16;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int16_t);
				int16_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "uint16")
			{
				varType = VarType::UINT16;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint16_t);
				uint16_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "int")
			{
				varType = VarType::INT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int32_t);
				int32_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "uint")
			{
				varType = VarType::UINT;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint32_t);
				uint32_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "long")
			{
				varType = VarType::INT64;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(int64_t);
				int64_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "ulong")
			{
				varType = VarType::UINT64;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(uint64_t);
				uint64_t varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "char")
			{
				varType = VarType::CHAR;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value
				bytes = sizeof(char);
				char varState; mono_field_get_value(classInstance, field, &varState);
				memcpy(cursor, &varState, bytes);
				cursor += bytes;
			}
			else if (typeName == "string")
			{
				varType = VarType::STRING;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Serialize the var value				
				MonoString* varState; mono_field_get_value(classInstance, field, &varState);
				char* cString = mono_string_to_utf8(varState);

				std::string defString(cString);

				bytes = sizeof(uint);

				uint stringLenght = defString.size();
				memcpy(cursor, &stringLenght, bytes);
				cursor += bytes;

				bytes = stringLenght;
				memcpy(cursor, defString.c_str(), bytes);
				cursor += bytes;

				mono_free(cString);
			}
			else if (typeName == "FlanEngine.GameObject")
			{
				varType = VarType::GAMEOBJECT;
									
				MonoObject* monoObject; mono_field_get_value(classInstance, field, &monoObject);

				GameObject* serializableGO = monoObject ? App->scripting->GameObjectFrom(monoObject) : nullptr;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Here save the UID of the gameObject you have referenced
				UID uid = serializableGO ? serializableGO->uuid : 0;
				bytes = sizeof(UID);
				memcpy(cursor, &uid, bytes);
				cursor += bytes;

			}
			else if (typeName == "FlanEngine.Transform")
			{
				varType = VarType::TRANSFORM;

				MonoObject* transformObj; mono_field_get_value(classInstance, field, &transformObj);

				MonoObject* monoObject;

				transformObj ? mono_field_get_value(transformObj, mono_class_get_field_from_name(mono_object_get_class(transformObj), "gameObject"), &monoObject) : monoObject = nullptr;

				GameObject* serializableGO = monoObject ? App->scripting->GameObjectFrom(monoObject) : nullptr;

				//Serialize the varType
				bytes = sizeof(varType);
				memcpy(cursor, &varType, bytes);
				cursor += bytes;

				//Serialize the varName (lenght + string)

				bytes = sizeof(uint);
				uint nameLenght = fieldName.length();
				memcpy(cursor, &nameLenght, bytes);
				cursor += bytes;

				bytes = nameLenght;
				memcpy(cursor, fieldName.c_str(), bytes);
				cursor += bytes;

				//Here save the UID of the transform->gameObject you have referenced
				UID uid = serializableGO ? serializableGO->uuid : 0;
				bytes = sizeof(UID);
				memcpy(cursor, &uid, bytes);
				cursor += bytes;
			}
		}
		field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
	}
}

void ComponentScript::deSerializePublicVars(char *& cursor)
{
	if (!classInstance)
		return ;

	uint numVars = 0;
	uint bytes = sizeof(uint);
	memcpy(&numVars, cursor, bytes);
	cursor += bytes;

	for (int i = 0; i < numVars; i++)
	{
		//deSerialize type
		VarType varType;
		uint bytes = sizeof(VarType);
		memcpy(&varType, cursor, bytes);
		cursor += bytes;

		//deSerialize lenght + string
		bytes = sizeof(uint);
		uint nameLenght;
		memcpy(&nameLenght, cursor, bytes);
		cursor += bytes;

		bytes = nameLenght;
		std::string varName;
		varName.resize(nameLenght);
 		memcpy((void*)varName.c_str(), cursor, bytes);
		varName.resize(nameLenght);		//TODO: Check if names are deSerializing well with this resize
		cursor += bytes;	

		//deSerialize data
		switch (varType)
		{
		case VarType::BOOL:
		{
			bytes = sizeof(bool);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "bool" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::FLOAT:
		{
			bytes = sizeof(float);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "single" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::DOUBLE:
		{
			bytes = sizeof(double);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "double" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::INT8:
		{
			bytes = sizeof(signed char);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "sbyte" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT8:
		{
			bytes = sizeof(unsigned char);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "byte" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::INT16:
		{
			bytes = sizeof(short);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "int16" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT16:
		{
			bytes = sizeof(unsigned short);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "uint16" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::INT:
		{
			bytes = sizeof(int);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "int" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT:
		{
			bytes = sizeof(uint);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "uint" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::INT64:
		{
			bytes = sizeof(long long);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "long" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::UINT64:
		{
			bytes = sizeof(unsigned long long);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "ulong" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}		
		case VarType::CHAR:
		{
			bytes = sizeof(char);
			bool var;
			memcpy(&var, cursor, bytes);
			cursor += bytes;
			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "char" && fieldName == varName)
					{
						mono_field_set_value(classInstance, field, &var);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}
			break;
		}
		case VarType::STRING:
		{
			bytes = sizeof(uint);
			uint stringLength;
			memcpy(&stringLength, cursor, bytes);
			cursor += bytes;

			std::string string;
			string.resize(stringLength);
			memcpy((void*)string.c_str(), cursor, bytes);
			string.resize(stringLength);		//TODO: Check if names are deSerializing well with this resize
			cursor += bytes;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "string" && fieldName == varName)
					{
						MonoString* monoString = mono_string_new(App->scripting->domain,string.c_str());
						mono_field_set_value(classInstance, field, monoString);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}

			break;
		}
		case VarType::GAMEOBJECT:	
		{
			bytes = sizeof(UID);
			UID uid;
			memcpy(&uid, cursor, bytes);
			cursor += bytes;
			
			GameObject* go = nullptr;

			if (uid != 0)
			{
				go = App->resources->FindPrefabGObyID(uid);

				if (!go)
				{
					go = App->scene->FindGameObjectByID(uid);
					if (!go)
					{
						Debug.LogError("A Script lost a Gameobject reference");
					}
				}
			}

			MonoObject* monoObject = go ? App->scripting->MonoObjectFrom(go) : nullptr;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

					if (typeName == "FlanEngine.GameObject" && fieldName == varName)
					{
						
						mono_field_set_value(classInstance, field, monoObject);
						break;
					}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}

			break;
		}
		case VarType::TRANSFORM:
		{
			bytes = sizeof(UID);
			UID uid;
			memcpy(&uid, cursor, bytes);
			cursor += bytes;

			GameObject* go = nullptr;

			if (uid != 0)
			{
				go = App->resources->FindPrefabGObyID(uid);

				if (!go)
				{
					go = App->scene->FindGameObjectByID(uid);
					if (!go)
					{
						Debug.LogError("A Script lost a Transform reference");
					}
				}
			}

			MonoObject* monoObject = go ? App->scripting->MonoObjectFrom(go) : nullptr;

			void* iterator = 0;
			MonoClassField* field = mono_class_get_fields(mono_object_get_class(classInstance), &iterator);

			while (field != nullptr)
			{
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC && !(flags & MONO_FIELD_ATTR_STATIC))
				{
					MonoType* type = mono_field_get_type(field);
					std::string typeName = mono_type_full_name(type);
					std::string fieldName = mono_field_get_name(field);

						if (typeName == "FlanEngine.Transform" && fieldName == varName)
						{
							if (monoObject)
							{
								MonoObject* monoTransform;
								mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "transform"), &monoTransform);
								mono_field_set_value(classInstance, field, monoTransform);
							}
							else
							{
								mono_field_set_value(classInstance, field, nullptr);
							}
							break;
						}
				}
				field = mono_class_get_fields(mono_object_get_class(classInstance), (void**)&iterator);
			}

			break;
		}
		default:
			break;
		}
	}	
}

void ComponentScript::InstanceClass()
{
	if (!scriptRes || scriptRes->state != ResourceScript::ScriptState::COMPILED_FINE)
		return;

	MonoClass* klass = mono_class_from_name(scriptRes->image, "", scriptName.data());
	classInstance = mono_object_new(App->scripting->domain, klass);
	
	mono_runtime_object_init(classInstance);

	//Reference the gameObject var with the MonoObject relative to this GameObject
	MonoObject* monoGO = App->scripting->MonoObjectFrom(gameObject);

	//SetUp this monoGO inside the class Instance
	MonoClassField* instanceMonoGo = mono_class_get_field_from_name(klass, "gameObject");
	mono_field_set_value(classInstance, instanceMonoGo, monoGO);

	//Create the handle storage to make sure the garbage collector doesn't delete the classInstance
	handleID = mono_gchandle_new(classInstance, true);
}
