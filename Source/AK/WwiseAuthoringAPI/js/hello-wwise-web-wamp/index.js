var showMessage = function(message){
  document.getElementById("message").innerHTML = message; 
}

function onBodyLoad() {
    // Create the WAMP connection
    var connection = new autobahn.Connection({
            url: 'ws://localhost:8080/waapi',
            realm: 'realm1',
            protocols: ['wamp.2.json']
        });

    // Setup handler for connection closed
    connection.onclose = function (reason, details) {
        showMessage('wamp connection closed');
        return true;
    };

    // Setup handler for connection opened
    connection.onopen = function (session) {
        showMessage('wamp connection opened');

        // Call getInfo
        session.call(ak.wwise.core.getInfo, [], {}).then(
            function (res) {
                showMessage(`Hello ${res.kwargs.displayName} ${res.kwargs.version.displayName}`);
            },
            function (error) {
                showMessage(`error: ${error}`);
            }        
        );
    };

    // Open the connection
    connection.open();
}
