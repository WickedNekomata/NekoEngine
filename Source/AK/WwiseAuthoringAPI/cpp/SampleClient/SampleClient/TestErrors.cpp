/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Version: v2017.2.6  Build: 6636
Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#include "TestErrors.h"

#include "AK/WwiseAuthoringAPI/AkAutobahn/Client.h"
#include "AK/WwiseAuthoringAPI/AkAutobahn/FutureUtils.h"
#include "AK/WwiseAuthoringAPI/AkAutobahn/Logger.h"
#include "RapidJsonUtils.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <AK/WwiseAuthoringAPI/waapi.h>

#include <string>
#include <iostream>
#include <sstream>
#include <codecvt>

void TestErrors()
{
	using namespace AK::WwiseAuthoringAPI;

	{
		Client client;

		std::cout << "Testing with AkJson...\n\n";

		// Using the client without connecting first.
		{
			std::cout << "Calling without connecting first: ";

			AkJson resultJson;
			if (!client.Call(ak::wwise::core::getInfo, AkJson(AkJson::Type::Map), AkJson(AkJson::Type::Map), resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Connecting, should not fail.
		{
			if (!client.Connect("127.0.0.1", 8080))
			{
				std::cout << "Unexpected error: Could not connect to Wwise Authoring on localhost." << std::endl;
				return;
			}
		}

		// Calling with the wrong URI.
		{
			std::cout << "Calling with the wrong URI: ";

			AkJson resultJson;
			if (!client.Call("ak.wwise.wronguri", AkJson(AkJson::Type::Map), AkJson(AkJson::Type::Map), resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Calling and failing schema validation.
		{
			std::cout << "Failing schema validation: ";

			AkJson resultJson;
			if (!client.Call(ak::wwise::core::getInfo, AkJson(AkJson::Map{ { "invalidKey", AkVariant() } }), AkJson(AkJson::Type::Map), resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Subscribing with the wrong URI.
		{
			std::cout << "Subscribing with the wrong URI: ";

			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto callback = [](uint64_t, const JsonProvider&)
			{
			};

			// Subscribe to object created notification
			AkJson resultJson;
			uint64_t idSubscription;
			if (!client.Subscribe("ak.wwise.invaliduri", AkJson(AkJson::Type::Map), callback, idSubscription, resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Subscribing and failing schema validation.
		{
			std::cout << "Subscribing and failing schema validation: ";

			auto callback = [](uint64_t, const JsonProvider&)
			{
			};

			// Subscribe to object created notification
			AkJson resultJson;
			uint64_t idSubscription;
			if (!client.Subscribe(ak::wwise::core::object::created, AkJson(AkJson::Map{ { "invalidKey", AkVariant() } }), callback, idSubscription, resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Unsubscribing with the wrong subscription id.
		{
			std::cout << "Unsubscribing with the wrong subscription id: ";

			const uint64_t badSubscriptionId = 0xFFFFFFFFFFFFFFFF;

			AkJson resultJson;
			if (!client.Unsubscribe(badSubscriptionId, resultJson))
			{
				std::cout << RapidJsonUtils::GetAkJsonString(resultJson) << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}
	}
	
	{
		Client client;

		std::cout << "\nTesting with JSON string...\n\n";

		// Using the client without connecting first.
		{
			std::cout << "Calling without connecting first: ";

			std::string resultJson;
			if (!client.Call(ak::wwise::core::getInfo, "{}", "{}", resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}
		
		// Connecting, should not fail.
		{
			if (!client.Connect("127.0.0.1", 8080))
			{
				std::cout << "Unexpected error: Could not connect to Wwise Authoring on localhost." << std::endl;
				return;
			}
		}

		// Calling with the wrong URI.
		{
			std::cout << "Calling with the wrong URI: ";

			std::string resultJson;
			if (!client.Call("ak.wwise.wronguri", "{}", "{}", resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Calling and failing schema validation.
		{
			std::cout << "Failing schema validation: ";

			std::string resultJson;
			if (!client.Call(ak::wwise::core::getInfo, "{\"invalidKey\": 123}", "{}", resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Subscribing with the wrong URI.
		{
			std::cout << "Subscribing with the wrong URI: ";

			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto callback = [](uint64_t, const JsonProvider&)
			{
			};

			// Subscribe to object created notification
			std::string resultJson;
			uint64_t idSubscription;
			if (!client.Subscribe("ak.wwise.invaliduri", "{}", callback, idSubscription, resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}

		// Subscribing and failing schema validation.
		{
			std::cout << "Subscribing and failing schema validation: ";

			auto callback = [](uint64_t, const JsonProvider&)
			{
			};

			// Subscribe to object created notification
			std::string resultJson;
			uint64_t idSubscription;
			if (!client.Subscribe(ak::wwise::core::object::created, "{\"invalidKey\": 123}", callback, idSubscription, resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}
		
		// Unsubscribing with the wrong subscription id.
		{
			std::cout << "Unsubscribing with the wrong subscription id: ";

			const uint64_t badSubscriptionId = 0xFFFFFFFFFFFFFFFF;

			std::string resultJson;
			if (!client.Unsubscribe(badSubscriptionId, resultJson))
			{
				std::cout << resultJson << std::endl;
			}
			else
			{
				std::cout << "Expected an error." << std::endl;
				return;
			}
		}
	}
}
