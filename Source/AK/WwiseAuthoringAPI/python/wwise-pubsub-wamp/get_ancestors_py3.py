import asyncio
import sys
import os

from autobahn.asyncio.wamp import ApplicationRunner
from ak_autobahn import AkComponent

# You may also copy-paste the waapi.py file alongside this sample
sys.path.append(os.path.join(os.path.dirname(__file__), '../../../../include/AK/WwiseAuthoringAPI/py'))
from waapi import WAAPI_URI

done = False

class MyComponent(AkComponent):
    def onJoin(self, details):
        def on_object_created(**kwargs):
            result = kwargs[u"object"]

            print("The object was created in the category: {}".format(result[u"category"]))

            arguments = {
                "from": {"id": [result.get("id")]},
                "transform": [{"select": ["ancestors"]}],
                "options": {
                    "return": ["name"]
                }
            }

            res = yield from self.call(WAAPI_URI.ak_wwise_core_object_get, **arguments)

            ancestors = res.kwresults[u"return"]
            print(u"Ancestor of {}:".format(result[u"id"]))
            for ancestor in ancestors:
                print("\t{}".format(ancestor[u"name"]))
            
            global done
            if not done:
                self.leave() # Disconnect
            done = True

        subscribe_args = {
            "options": {
                "return": ["id", "name", "category"]
            }
        }

        # Subscribe to ak.wwise.core.object.created
        # Calls on_object_created whenever the event is received 
        yield from self.subscribe(on_object_created,
                                  WAAPI_URI.ak_wwise_core_object_created,
                                  **subscribe_args)

        print("Create an object in the Project Explorer")

    def onDisconnect(self):
        print("The client was disconnected.")
        asyncio.get_event_loop().stop()

if __name__ == '__main__':
    runner = ApplicationRunner(url=u"ws://127.0.0.1:8080/waapi", realm=u"get_ancestors_demo")
    try:
        runner.run(MyComponent)
    except Exception as e:
        print(type(e).__name__ + ": Is Wwise running and Wwise Authoring API enabled?")
