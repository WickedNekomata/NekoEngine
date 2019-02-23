

angular.module('app', [])
    .controller('SelectionController', function ($timeout) {

        // Data
        var ctrl = this;
        ctrl.status = 'Initializing...';
        ctrl.err = null;
        ctrl.objects = [];

        ctrl.wampsession = null;

        // Create the WAMP connection
        ctrl.connection = new autobahn.Connection({
            url: 'ws://localhost:8080/waapi',
            realm: 'realm1',
            protocols: ['wamp.2.json']
        });

        // Update the selection
        ctrl.updateSelection = function (res) {
            $timeout(function () {
                ctrl.objects = res.kwargs.objects;
            });
        };

        ctrl.connection.onopen = function (session) {
            $timeout(function () {
                ctrl.status = 'connected';

                wampsession = session;

                // Calling getInfo in order to get the current Wwise version through WAAPI
                wampsession.call(ak.wwise.core.getInfo, [], {}).then(
                    function (res) {
                        $timeout(function(){
                            ctrl.status = `connected to ${res.kwargs.displayName} ${res.kwargs.version.displayName}`;
                        });
                        
                    },
                    ctrl.onError);

                // Initial update
                wampsession.call(ak.wwise.ui.getSelectedObjects , [], {}, {return:['name', 'notes', '@Volume']}).then(
                    ctrl.updateSelection,
                    ctrl.onError);

                // Subcribe to selection changes
                wampsession.subscribe(ak.wwise.ui.selectionChanged, function(args, kwargs, details) {
                    // Received a selection changed, query objects again
                    wampsession.call(ak.wwise.ui.getSelectedObjects , [], {}, {return:['name', 'notes', '@Volume']}).then(
                        ctrl.updateSelection,
                        ctrl.onError);                    
                }).then(
                    function(){},
                    ctrl.onError);                
            })
        };

        ctrl.connection.onclose = function(){
            $timeout(function(){
                ctrl.status = `disconnected. Press F5 to reconnect.`;
                ctrl.objects = [];
            });            
        }

        ctrl.connection.open();

        // General handler for errors
        ctrl.onError = function(err) {
            $timeout(function(){
                ctrl.err = `Error: ${JSON.stringify(err,null, 4)}`;
                ctrl.objects = [];
            });
        };
    });
