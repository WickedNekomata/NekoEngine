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

#include "AK/WwiseAuthoringAPI/AkAutobahn/Client.h"

#include <iostream>
#include <string>
#include <sstream>

#include "JSONHelpers.h"
#include "AK/WwiseAuthoringAPI/AkAutobahn/Logger.h"

namespace AK
{
	namespace WwiseAuthoringAPI
	{
		Client::Client()
		{
			m_ws = new session();
		}

		Client::~Client()
		{
			delete m_ws;

			// IMPORTANT: In theory, deleting the websocket will hang the thread until the receiver thread has exited,
			// thus giving the garantee that the map "m_subscribers" can then be safely disposed of when the Client
			// object is fully destructed.
		}

		void Client::Log(const char* log)
		{
			Logger::Get()->LogMessage("WwiseAuthoringAPI client", log);
		}

		bool Client::Connect(const char* in_uri, unsigned int in_port)
		{
			try
			{
				m_ws->start(in_uri, in_port);
				auto future = m_ws->join("realm1");
				uint64_t result;
				return GetFuture<uint64_t>(future, -1, result);
			}
			catch (const std::exception& e)
			{
				Log(e.what());
				return false;
			}
		}
		
        bool Client::IsConnected() const
        {
            if (m_ws != nullptr)
                return m_ws->isConnected();
            return false;
        }


		void Client::Disconnect()
		{
			m_ws->stop(std::make_exception_ptr(connection_error("connection closed by destruction of session")));
		}
		
		bool Client::Call(const char* in_uri, const char* in_args, const char* in_options, std::string& out_result, int in_timeoutMs)
		{
			try
			{
				AkJson jsonArgs;
				rapidjson::Document docArgs;

				if (docArgs.Parse(in_args).HasParseError() || !JSONHelpers::FromRapidJson(docArgs, jsonArgs))
				{
					throw std::runtime_error("in_args should contain a valid JSON object string (empty object is allowed).");
				}

				AkJson jsonOptions;
				rapidjson::Document docOptions;

				if (docOptions.Parse(in_options).HasParseError() || !JSONHelpers::FromRapidJson(docOptions, jsonOptions))
				{
					throw std::runtime_error("in_options should contain a valid JSON object string (empty object is allowed).");
				}
				
				AkJson result;
				auto future = m_ws->call_options(in_uri, std::vector<AkVariant>{}, jsonArgs, jsonOptions);
				bool success = GetFuture<AkJson>(future, in_timeoutMs, result);

				if (!success)
				{
					return false;
				}

				out_result = JSONHelpers::GetAkJsonString(result);
				return true;
			}
			catch (std::exception& e)
			{
				Log(e.what());
				AkJson errorJson;
				ErrorToAkJson(e, errorJson);
				out_result = JSONHelpers::GetAkJsonString(errorJson);
				return false;
			}
		}
		
		void Client::ErrorToAkJson(const std::exception& in_exception, AkJson& out_result)
		{
			rapidjson::Document docError;

			if (!docError.Parse(in_exception.what()).HasParseError())
			{
				JSONHelpers::FromRapidJson(docError, out_result);
			}
			else
			{
				out_result = AkJson(AkJson::Map{
					{ "message", AkVariant(in_exception.what()) }
				});
			}
		}
		
		bool Client::Call(const char* in_uri, const AkJson& in_args, const AkJson& in_options, AkJson& out_result, int in_timeoutMs)
		{
			try
			{
				auto future = m_ws->call_options(in_uri, std::vector<AkVariant>{}, in_args, in_options);
				return GetFuture<AkJson>(future, in_timeoutMs, out_result);
			}
			catch (std::exception& e)
			{
				Log(e.what());
				ErrorToAkJson(e, out_result);
				return false;
			}
		}

		bool Client::SubscribeImpl(const char* in_uri, const AkJson& in_options, handler_t in_callback, int in_timeoutMs, uint64_t& out_subscriptionId)
		{
			Client* pThis = this;

			auto future = m_ws->subscribe(in_uri, in_callback, in_options);

			// wait for the answer
			subscription resultObject;
			if (!GetFuture<subscription>(future, in_timeoutMs, resultObject))
			{
				return false;
			}

			out_subscriptionId = resultObject.id;
			return true;
		}

		bool Client::Subscribe(const char* in_uri, const char* in_options, WampEventCallback in_callback, uint64_t& out_subscriptionId, std::string& out_result, int in_timeoutMs)
		{
			try
			{
				rapidjson::Document doc;
				
				if (doc.Parse(in_options).HasParseError())
				{
					return false;
				}
				
				AkJson jsonOptions;
				JSONHelpers::FromRapidJson(doc, jsonOptions);

				return SubscribeImpl(in_uri, jsonOptions, in_callback, in_timeoutMs, out_subscriptionId);
			}
			catch (const std::exception& e)
			{
				Log(e.what());
				AkJson errorJson;
				ErrorToAkJson(e, errorJson);
				out_result = JSONHelpers::GetAkJsonString(errorJson);
			}

			return false;
		}
		
		bool Client::Subscribe(const char* in_uri, const AkJson& in_options, WampEventCallback in_callback, uint64_t& out_subscriptionId, AkJson& out_result, int in_timeoutMs)
		{
			try
			{
				return SubscribeImpl(in_uri, in_options, in_callback, in_timeoutMs, out_subscriptionId);
			}
			catch (const std::exception& e)
			{
				Log(e.what());
				ErrorToAkJson(e, out_result);
			}

			return false;
		}
		
		bool Client::UnsubscribeImpl(const uint64_t& in_subscriptionId, int in_timeoutMs)
		{
			auto result = m_ws->unsubscribe(in_subscriptionId);

			AkJson output;
			return GetFuture<AkJson>(result, in_timeoutMs, output);
		}
		
		bool Client::Unsubscribe(const uint64_t& in_subscriptionId, std::string& out_result, int in_timeoutMs)
		{
			try
			{
				return UnsubscribeImpl(in_subscriptionId, in_timeoutMs);
			}
			catch (const std::exception& e)
			{
				Log(e.what());
				AkJson errorJson;
				ErrorToAkJson(e, errorJson);
				out_result = JSONHelpers::GetAkJsonString(errorJson);
			}

			return false;
		}

		bool Client::Unsubscribe(const uint64_t& in_subscriptionId, AkJson& out_result, int in_timeoutMs)
		{
			try
			{
				return UnsubscribeImpl(in_subscriptionId, in_timeoutMs);
			}
			catch (const std::exception& e)
			{
				Log(e.what());
				ErrorToAkJson(e, out_result);
			}

			return false;
		}
	}
}
