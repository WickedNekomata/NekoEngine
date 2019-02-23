var ak = require('../../../../include/AK/WwiseAuthoringAPI/js/waapi.js').ak;
var autobahn = require('autobahn');

// Create the WAMP connection
var connection = new autobahn.Connection({
        url: 'ws://localhost:8080/waapi',
        realm: 'realm1',
        protocols: ['wamp.2.json']
    });

// Setup handler for connection opened
connection.onopen = function (session) {

    // Call getInfo
    session.call(ak.wwise.core.getInfo, [], {}).then(
        function (res) {
            console.log(`Hello ${res.kwargs.displayName} ${res.kwargs.version.displayName}!`);
        },
        function (error) {
            console.log(`Error: ${error}`);
        }        
    ).then(
        function() {
            connection.close();
        }
    );
};

connection.onclose = function (reason, details) {
    if (reason !== 'lost') {
        console.log("Connection closed. Reason: " + reason);
    }
    process.exit();
};

// Open the connection
connection.open();
