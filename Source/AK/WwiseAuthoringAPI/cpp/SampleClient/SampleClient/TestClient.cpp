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

#include "TestClient.h"

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

static void logCout(const char* log)
{
	std::cout << log << std::endl;
}

void HelloWorld()
{
	using namespace AK::WwiseAuthoringAPI;

	// Enable this for debugging purposes.
	//Logger::Get()->SetLoggerFunction(logCout);

	Client client;

	// Connect to Wwise Authoring on localhost.

	if (!client.Connect("127.0.0.1", 8080))
	{
		std::cout << "Could not connect to Wwise Authoring on localhost." << std::endl;
		return;
	}

	// Obtain the wwise info.
	AkJson wwiseInfoJson;
	if (!client.Call(ak::wwise::core::getInfo, AkJson(AkJson::Type::Map), AkJson(AkJson::Type::Map), wwiseInfoJson))
	{
		std::cout << "Failed to obtain Wwise Info: " << std::string(wwiseInfoJson["message"].GetVariant()) << std::endl;
		return;
	}
	
	// Output some information, including converting from UTF8 for proper display.
	std::ostringstream os;
	os << wwiseInfoJson["displayName"].GetVariant().GetString() << " " << wwiseInfoJson["version"]["displayName"].GetVariant().GetString() << "\n";
	os << wwiseInfoJson["copyright"].GetVariant().GetString() << "\n";
	os << wwiseInfoJson["branch"].GetVariant().GetString() << "\n\n";

#if defined (_WIN32) || defined (_WIN64)
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(os.str());
	std::wcout << wstr;
#else
	std::cout << os.str();
#endif
}

void TestWampClient()
{
	using namespace AK::WwiseAuthoringAPI;

	// Enable this for debugging purposes.
	//Logger::Get()->SetLoggerFunction(logCout);

	{
		Client client;

		if (!client.Connect("127.0.0.1", 8080))
		{
			std::cout << "Please make sure Wwise Authoring is running and try again." << std::endl;
			return;
		}
		
		uint64_t idObjectCreatedSubscription = 0;
		std::future<bool> objectCreatedFuture;
		std::promise<bool> objectCreatedPromise;
		std::atomic_bool objectCreatedCalled;

		{
			objectCreatedCalled = false;
			objectCreatedFuture = objectCreatedPromise.get_future();

			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto objectCreated = [&objectCreatedCalled, &objectCreatedPromise](uint64_t, const JsonProvider& in_rJson)
			{
				// We can't assume exclusive usage of the server, therefore we can't assume events will be fired only once.
				// Let's make sure we don't execute this more than once.
				if (!objectCreatedCalled)
				{
					const char* szName = in_rJson.GetAkJson()["object"]["id"].GetVariant().GetString().c_str();
					std::cout << "Received object notification. Object id is " << szName << std::endl;
					objectCreatedCalled = true;
					objectCreatedPromise.set_value(true);
				}
			};

			// Subscribe to object created notification
			AkJson subscribeResult;
			client.Subscribe(ak::wwise::core::object::created, AkJson(AkJson::Type::Map), objectCreated, idObjectCreatedSubscription, subscribeResult);
		}

		AkJson queryResult;

		{
			// Get the default work unit for actor-mixer
			AkJson args(AkJson::Map{
				{ "from", AkJson::Map{
					{ "path", AkJson::Array{ AkVariant("\\Actor-Mixer Hierarchy\\Default Work Unit") } } } }
				});

			AkJson options(AkJson::Map{ 
				{ "return", AkJson::Array{ AkVariant("id"), AkVariant("name"), AkVariant("category") } }
			});

			if (!client.Call(ak::wwise::core::object::get, args, options, queryResult))
			{
				std::cout << "Failed to get the default actor mixer work unit" << std::endl;
				return;
			}
		}

		AkJson createResult;

		{
			// Get the query results
			const auto& firstObject = queryResult["return"].GetArray()[0];
			const auto& parentId = firstObject["id"];

			// create an object
			AkJson args(AkJson::Map{
				{ "parent", parentId },
				{ "type", AkVariant("Sound") },
				{ "name", AkVariant("Boom") },
				{ "onNameConflict", AkVariant("rename") },
			});

			if (!client.Call(ak::wwise::core::object::create, args, AkJson(AkJson::Type::Map), createResult))
			{
				std::cout << "Failed to create an object" << std::endl;
				return;
			}
		}

		// Retreive the object id created
		const auto& createId = createResult["id"];
		uint64_t idPropertyChangedSubscription = 0;
		std::promise<bool> propertyChangedPromise;
		std::future<bool> propertyChangedFuture;
		std::atomic_bool propertyChangedCalled;

		{
			// Subscribe to property changed.
			AkJson options(AkJson::Map
			{
				{ "object", createId.GetVariant() },
				{ "property", AkVariant("Volume") },
				{ "return", AkJson::Array{
					AkVariant("id")
				} }
			});
			
			propertyChangedCalled = false;
			propertyChangedFuture = propertyChangedPromise.get_future();

			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto onPropertyChanged = [&propertyChangedCalled, &propertyChangedPromise](uint64_t subscriptionId, const JsonProvider&)
			{
				// We can't assume exclusive usage of the server, therefore we can't assume events will be fired only once.
				// Let's make sure we don't execute this more than once.
				if (!propertyChangedCalled)
				{
					std::cout << "Received property changed event. Subscription id is " << subscriptionId << std::endl;
					propertyChangedCalled = true;
					propertyChangedPromise.set_value(true);
				}
			};

			AkJson subscribeResult;
			if (!client.Subscribe(ak::wwise::core::object::propertyChanged, options, onPropertyChanged, idPropertyChangedSubscription, subscribeResult))
			{
				std::cout << "Failed to subscribe to propertyChanged." << std::endl;
				return;
			}
			else
			{
				std::cout << "Subscribed to propertyChanged. Subscription id is " << idPropertyChangedSubscription << std::endl;
			}
		}

		{
			// Set the property.
			AkJson args(AkJson::Map{
				{ "object", createId.GetVariant() },
				{ "platform", AkVariant("{00000000-0000-0000-0000-000000000000}") },
				{ "property", AkVariant("Volume") },
				{ "value", AkVariant(0.1) },
			});

			AkJson result;
			if (!client.Call(ak::wwise::core::object::setProperty, args, AkJson(AkJson::Type::Map), result))
			{
				std::cout << "Failed to create an object" << std::endl;
				return;
			}
		}
		
		// Block here until both events have been received.
		propertyChangedFuture.get();
		objectCreatedFuture.get();

		AkJson unsubscribeResult;
		client.Unsubscribe(idPropertyChangedSubscription, unsubscribeResult);
		client.Unsubscribe(idObjectCreatedSubscription, unsubscribeResult);
	}

	// Connection closes when the client goes out of scope.

	std::cout << "Done!" << std::endl;
}

void TestWampClientJsonString()
{
	using namespace AK::WwiseAuthoringAPI;

	// Enable this for debugging purposes.
	//Logger::Get()->SetLoggerFunction(logCout);

	{
		Client client;

		if (!client.Connect("127.0.0.1", 8080))
		{
			std::cout << "Please make sure Wwise Authoring is running and try again." << std::endl;
			return;
		}

		uint64_t idObjectCreatedSubscription = 0;
		std::future<bool> objectCreatedFuture;
		std::promise<bool> objectCreatedPromise;
		std::atomic_bool objectCreatedCalled;

		{
			objectCreatedCalled = false;
			objectCreatedFuture = objectCreatedPromise.get_future();

			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto objectCreated = [&objectCreatedCalled, &objectCreatedPromise](uint64_t, const JsonProvider& in_rJson)
			{
				// We can't assume exclusive usage of the server, therefore we can't assume events will be fired only once.
				// Let's make sure we don't execute this more than once.
				if (!objectCreatedCalled)
				{
					std::cout << "Received object notification. JSON string is " << in_rJson.GetJsonString() << std::endl;
					objectCreatedCalled = true;
					objectCreatedPromise.set_value(true);
				}
			};

			// Subscribe to object created notification
			AkJson subscribeResult;
			client.Subscribe(ak::wwise::core::object::created, AkJson(AkJson::Type::Map), objectCreated, idObjectCreatedSubscription, subscribeResult);
		}

		AkJson queryResult;

		{
			std::string queryResultString;

			// Get the default work unit for actor-mixer
			std::string args = "\n{\"from\":\n\n{\"path\":\n[\t\"\\\\Actor-Mixer Hierarchy\\\\Default Work Unit\"\r]\r\n}}\t\r\n";

			std::string options = "{\n\"return\"\n:\n\n[\t\"id\",\"name\",\"category\"]\t}\r";

			if (!client.Call(ak::wwise::core::object::get, args.c_str(), options.c_str(), queryResultString))
			{
				std::cout << "Failed to get the default actor mixer work unit" << std::endl;
				return;
			}

			rapidjson::Document doc;
			if (doc.Parse(queryResultString.c_str()).HasParseError() || !RapidJsonUtils::FromRapidJson(doc, queryResult))
			{
				std::cout << "Failed to parse result JSON string." << std::endl;
				return;
			}
		}
		
		AkJson createResult;

		{
			std::string createResultString;

			// Get the query results
			const auto& firstObject = queryResult["return"].GetArray()[0];
			const auto& parentId = firstObject["id"];

			std::string args = "{\"name\":\"Boom\",\"onNameConflict\":\"rename\",\"parent\":\"" + parentId.GetVariant().GetString() + "\",\"type\":\"Sound\"}";

			if (!client.Call(ak::wwise::core::object::create, args.c_str(), "{}", createResultString))
			{
				std::cout << "Failed to create an object" << std::endl;
				return;
			}

			rapidjson::Document doc;
			if (doc.Parse(createResultString.c_str()).HasParseError() || !RapidJsonUtils::FromRapidJson(doc, createResult))
			{
				std::cout << "Failed to parse result JSON string." << std::endl;
				return;
			}
		}
		
		// Retreive the object id created
		const auto& createId = createResult["id"];
		uint64_t idPropertyChangedSubscription = 0;
		std::promise<bool> propertyChangedPromise;
		std::future<bool> propertyChangedFuture;
		std::atomic_bool propertyChangedCalled;
		
		{
			// Subscribe to property changed.
			std::string options = "{\"object\":\"" + std::string(createId.GetVariant()) + "\",\"property\":\"Volume\",\"return\":[\"id\"]}";

			propertyChangedCalled = false;
			propertyChangedFuture = propertyChangedPromise.get_future();
			
			// This assumes that we block the thread until events have been received, and then unsubscribe before going out of scope.
			auto onPropertyChanged = [&propertyChangedCalled, &propertyChangedPromise](uint64_t subscriptionId, const JsonProvider&)
			{
				// We can't assume exclusive usage of the server, therefore we can't assume events will be fired only once.
				// Let's make sure we don't execute this more than once.
				if (!propertyChangedCalled)
				{
					std::cout << "Received property changed event. Subscription id is " << subscriptionId << std::endl;
					propertyChangedCalled = true;
					propertyChangedPromise.set_value(true);
				}
			};

			std::string subscribeResult;
			if (!client.Subscribe(ak::wwise::core::object::propertyChanged, options.c_str(), onPropertyChanged, idPropertyChangedSubscription, subscribeResult))
			{
				std::cout << "Failed to subscribe to propertyChanged" << std::endl;
				return;
			}
			else
			{
				std::cout << "Subscribed to propertyChanged. Subscription id is " << idPropertyChangedSubscription << std::endl;
			}
		}

		{
			// Set the property.
			std::string args = "{\"object\":\"" + std::string(createId.GetVariant()) + "\",\"platform\":\"{00000000-0000-0000-0000-000000000000}\",\"property\":\"Volume\",\"value\":0.1}";

			std::string result;
			if (!client.Call(ak::wwise::core::object::setProperty, args.c_str(), "{}", result))
			{
				std::cout << "Failed to create an object" << std::endl;
				return;
			}
		}

		// Block here until both events have been received.
		propertyChangedFuture.get();
		objectCreatedFuture.get();

		AkJson unsubscribeResult;
		client.Unsubscribe(idPropertyChangedSubscription, unsubscribeResult);
		client.Unsubscribe(idObjectCreatedSubscription, unsubscribeResult);

		std::cout << "Disconnecting." << std::endl;
		client.Disconnect();

		std::cout << "Attempting to use a disconnected client." << std::endl;

		std::string result;
		if (!client.Call(ak::wwise::core::object::setProperty, "{}", "{}", result))
		{
			std::cout << "Failed as expected." << std::endl;
		}
		else
		{
			std::cout << "Call should not have succeeded." << std::endl;
		}
	}

	// Connection closes when the client goes out of scope.

	std::cout << "Done!" << std::endl;
}

void PerfTest()
{
	using namespace AK::WwiseAuthoringAPI;

	Client client;

	if (!client.Connect("127.0.0.1", 8080))
	{
		std::cout << "Please make sure Wwise Authoring is running and try again." << std::endl;
		return;
	}

	const unsigned int k_uiNumberLoop = 10000;

	// Get the default work unit for actor-mixer
	const AkJson args(AkJson::Map{
		{ "from", AkJson::Map{
			{ "ofType", AkJson::Array{ AkVariant("Project") } } } }
	});

	const AkJson options(AkJson::Map{
		{ "return", AkJson::Array{ AkVariant("id"), AkVariant("name") } }
	});

	AkJson queryResult;

	auto begin = std::chrono::high_resolution_clock::now();

	for (unsigned int i = 0; i < k_uiNumberLoop; ++i)
	{
		if (!client.Call(ak::wwise::core::object::get, args, options, queryResult))
		{
			std::cout << "Failed to get the default actor mixer work unit" << std::endl;
			return;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> duration = end - begin;
	std::cout << "total:" << duration.count() << " ms, average per call:" << duration.count() / (double)k_uiNumberLoop << "ms\n";
}