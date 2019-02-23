// Copyright Audiokinetic Inc.

(function () {
    
    var Client = require('node-rest-client').Client;
    var ak = require('../../../../include/AK/WwiseAuthoringAPI/js/waapi.js').ak;
    
    var httpParams = {
        data: {
            uri: ak.wwise.core.getInfo,
            options: {},
            args: {}
        },
        headers: { "Content-Type": "application/json" }
    };

    var client = new Client();
    
    var req = client.post("http://localhost:8090/waapi", httpParams, function (objectPayload, response) {

        if (response.statusCode != 200) {

            if (response.headers["content-type"] == "application/json") {
                console.log(`Error: ${objectPayload.uri}: ${JSON.stringify(objectPayload)}`);
            } else {
                console.log(`Error: ${(new Buffer(objectPayload)).toString("utf8")}`);
            }
            
            return;
        }

        console.log(`Hello ${objectPayload.displayName} ${objectPayload.version.displayName}`);
    });
    
    req.on("error", function (err) {
        console.log(`Error: ${err.message}`);
    });
    
})();
