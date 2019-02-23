import sys
import os

import requests
import json
import codecs

sys.path.append(os.path.join(os.path.dirname(__file__), '../../../../include/AK/WwiseAuthoringAPI/py'))
from waapi import WAAPI_URI


payload = {
    'uri': WAAPI_URI.ak_wwise_core_getinfo,
    'options': {},
    'args': {}
}

try:
    r = requests.post("http://localhost:8090/waapi", data=json.dumps(payload))
    print(r.status_code, r.reason)
    print(r.text.encode('utf8', 'replace'))
except:
    print("Unable to connect to Waapi: Is Wwise running and Wwise Authoring API enabled?")
