using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using SystemEx;

using WampSharp.V2;
using WampSharp.V2.Client;
using WampSharp.V2.PubSub;
using WampSharp.Core.Serialization;
using WampSharp.V2.Core.Contracts;


namespace WaapiCS
{
    internal class SubscribeContext
    {
        // Used to dispose of the subscription, effectively sending an unsubscribe message
        public IAsyncDisposable unsubscribeDisposable = null;
    }

    internal class PubSubWwise
    {
        const string serverAddress = "ws://127.0.0.1:8080/waapi";
        const string selectionChangedTopic = "ak.wwise.core.object.childAdded";

        public static void Main(string[] args)
        {
            DefaultWampChannelFactory factory = new DefaultWampChannelFactory();
            IWampChannel channel = factory.CreateJsonChannel(serverAddress, "realm1");
            channel.Open().Wait();

            IWampTopicProxy topicProxy = channel.RealmProxy.TopicContainer.GetTopicByUri(selectionChangedTopic);
            MySubscribeOptions options = new MySubscribeOptions();

            // Set your options here
            options.@return = new string[] { "id" };

            SubscribeContext context = new SubscribeContext();
            topicProxy.Subscribe(new MySubscriber(context), options)
                .ContinueWith(t => context.unsubscribeDisposable = t.Result)
                .Wait();

            Console.WriteLine("Add a child to an entity in the Wwise Authoring application.");
            Console.ReadLine();
        }
    }

    internal class MySubscribeOptions : SubscribeOptions
    {
        // Add data members with name corresponding to the option fields (see reference documentation)
        //          vvvvvvvvvvvvvvv
        [DataMember(Name = "return")]
        public IEnumerable<string> @return { get; set; }

        // If a data member is not set, it will not be sent to WAAPI
        [DataMember(Name = "platform")]
        public string platform { get; set; }
    }

    internal class MySubscriber : IWampRawTopicClientSubscriber
    {
        private SubscribeContext mContext;

        public MySubscriber(SubscribeContext context)
        {
            mContext = context ?? throw new ArgumentNullException("Context cannot be null");
        }

        public void Event<TMessage>(IWampFormatter<TMessage> formatter, long publicationId, EventDetails details, TMessage[] arguments, IDictionary<string, TMessage> argumentsKeywords)
        {
            // TMessage is used to represent a serialized message type
            // We know by contract that the child element is another dictionary
            IDictionary<string, TMessage> childDict = formatter.Deserialize<IDictionary<string, TMessage>>(argumentsKeywords["child"]);
            // Again, we know by contract that a field containing a string is expected for the key "id"
            string id = formatter.Deserialize<string>(childDict["id"]);

            Console.WriteLine("A child was added, ID={0}", id);
            Console.WriteLine("Press any key to continue...");

            // Dispose to unsubscribe
            mContext.unsubscribeDisposable.DisposeAsync().Wait();
        }

        // Other method overloads are never used: WAAPI always sends keyword arguments
        public void Event<TMessage>(IWampFormatter<TMessage> formatter, long publicationId, EventDetails details) { }
        public void Event<TMessage>(IWampFormatter<TMessage> formatter, long publicationId, EventDetails details, TMessage[] arguments) { }
    }

}