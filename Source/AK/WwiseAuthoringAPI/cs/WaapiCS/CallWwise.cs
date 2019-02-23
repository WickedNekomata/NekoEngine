using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using WampSharp.Core.Serialization;
using WampSharp.V2;
using WampSharp.V2.Client;
using WampSharp.V2.Core.Contracts;
using WampSharp.V2.Rpc;

namespace WaapiCS
{
    class CallWwise
    {
        const string serverAddress = "ws://127.0.0.1:8080/waapi";

        public static void Main(string[] args)
        {
            DefaultWampChannelFactory factory = new DefaultWampChannelFactory();
            IWampChannel channel = factory.CreateJsonChannel(serverAddress, "realm1");
            channel.Open().Wait();

            IWampRealmProxy realmProxy = channel.RealmProxy;

            CallGetInfo(realmProxy);
            CallGetSelectedObjects(realmProxy);
            
            Console.ReadLine();
        }

        public static void CallGetInfo(IWampRealmProxy realmProxy)
        {
            Console.WriteLine("Calling 'ak.wwise.core.getInfo'");

            // Arguments are passed using keywordArguments with primitive values which are serialized as Json
            IDictionary<string, object> keywordArguments = new Dictionary<string, object>();

            realmProxy.RpcCatalog.Invoke(
                new AssertCallback(),
                new CallOptions(),
                "ak.wwise.core.getInfo",
                new object[] { }, // Volontarily empty, we use only keywordArguments
                keywordArguments);
        }

        public static void CallGetSelectedObjects(IWampRealmProxy realmProxy)
        {
            Console.WriteLine("Calling 'ak.wwise.ui.getSelectedObjects'");

            // Optional arguments are passed using a subclass of CallOptions providing data members as specified by the Wwise Authoring API
            MyCallOptions options = new MyCallOptions();
            options.@return = new string[] { "id", "name", "parent" };

            realmProxy.RpcCatalog.Invoke(
                new GetSelectedObjectsCallback(),
                options,
                "ak.wwise.ui.getSelectedObjects",
                new object[] { }, // Volontarily empty, we use only keywordArguments
                new Dictionary<string, object>()); // Volontarily empty, no keywordArguments are necessary for this API call
        }
    }

    public class AssertCallback : IWampRawRpcOperationClientCallback
    {
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details, TMessage[] arguments, IDictionary<string, TMessage> argumentsKeywords)
        {
            string name = argumentsKeywords["displayName"].ToString();
            string copyright = formatter.Deserialize<JToken>(argumentsKeywords["version"])["displayName"].ToString();
            Console.WriteLine("ak.wwise.core.getInfo: Hello {0} {1}", name, copyright);
        }

        // Other method overloads are never used: WAAPI always sends keyword arguments
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error, TMessage[] arguments, TMessage argumentsKeywords) {}
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error) {}
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error, TMessage[] arguments) {}
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details) {}
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details, TMessage[] arguments) {}
    }

    public class GetSelectedObjectsCallback : IWampRawRpcOperationClientCallback
    {
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details, TMessage[] arguments, IDictionary<string, TMessage> argumentsKeywords)
        {
            string prefix = "ak.wwise.ui.getSelectedObjects: ";
            Console.WriteLine(prefix + "Got selected object data!");

            IEnumerable<JToken> objects = formatter.Deserialize<IEnumerable<JToken>>(argumentsKeywords["objects"]);
            int selectedObjectsNum = objects.Count<JToken>();
            Console.WriteLine(prefix + "Got {0} object(s)!", selectedObjectsNum);

            if (selectedObjectsNum >= 1)
            {
                JToken firstObject = objects.First<JToken>();
                
                Console.WriteLine(prefix + "The first selected object is '{0}' ({1})",
                    firstObject["name"].ToString(),
                    firstObject["id"].ToString());

                JToken parent = firstObject["parent"];
                if (parent == null)
                {
                    Console.WriteLine(prefix + "It has no parent.");
                }
                else
                {
                    Console.WriteLine(prefix + "Its parent is '{0}' ({1})",
                        parent["name"].ToString(),
                        parent["id"].ToString());
                }
            }
            else
            {
                Console.WriteLine(prefix + "Select something and try again!");
            }
        }

        // Other method overloads are never used: WAAPI always sends keyword arguments
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error, TMessage[] arguments, TMessage argumentsKeywords) { }
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error) { }
        public void Error<TMessage>(IWampFormatter<TMessage> formatter, TMessage details, string error, TMessage[] arguments) { }
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details) { }
        public void Result<TMessage>(IWampFormatter<TMessage> formatter, ResultDetails details, TMessage[] arguments) { }
    }

    public class MyCallOptions : CallOptions
    {
        // Add data members with name corresponding to the option fields (see reference documentation)
        //          vvvvvvvvvvvvvvv
        [DataMember(Name = "return")]
        public IEnumerable<string> @return { get; set; }

        // If a data member is not set, it will not be sent to WAAPI
        [DataMember(Name = "platform")]
        public string platform { get; set; }
    }
}
