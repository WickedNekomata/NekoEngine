var marker = null; // Our draggable marker object
var wampsession = null, connection=null;
var data = null;
var currentXParam = null, currentYParam = null;
var dragging = false;
var gameParams = {}; // local storing of fetched GameParameters

// detecting the browser's supported renderer.
var renderer = new PIXI.autoDetectRenderer(600, 600, {backgroundColor : 0xd4d4d4, antialias: true});
var stage = new PIXI.Container();

function init() {
    $('#xypad').append(renderer.view);
    marker = new PIXI.Graphics();
    marker.beginFill(0x00549F);
    marker.drawCircle(0, 0, 10);
    marker.x = renderer.view.width/2
    marker.y = renderer.view.height/2;
    marker.endFill();
    marker.interactive = true // enable the marker to respond to mouse and touch events
    marker.buttonMode = true // display the hand cursor on hover
    marker.on('pointerdown', onDragStart)
        .on('pointerup', onDragEnd)
        .on('pointerupoutside', onDragEnd)
        .on('pointermove', onDragMove);
    stage.addChild(marker);
    renderer.render(stage);

    $('#xParams').change(function(){
        if($(this).val()){
            currentXParam = gameParams[$(this).val()];
            marker.x = renderer.view.width*toRatio(currentXParam['@Min'],currentXParam['@Max'],currentXParam['@SimulationValue']);
            $('#xValue').text(currentXParam['@SimulationValue']);
            renderer.render(stage);
        }
    });

    $('#yParams').change(function(){
        if ($(this).val()){
            currentYParam = gameParams[$(this).val()];
            marker.y = renderer.view.height*(1-toRatio(currentYParam['@Min'],currentYParam['@Max'],currentYParam['@SimulationValue']));
            $('#yValue').text(currentYParam['@SimulationValue']);
            renderer.render(stage);
        }
    });
    // Create the WAMP connection
    connection = new autobahn.Connection({
        url: 'ws://localhost:8080/waapi',
        realm: 'realm1',
        protocols: ['wamp.2.json']
    });
    connection.onopen = function(session) {
        wampsession = session;
        // Calling getInfo in order to get the current Wwise version through WAAPI
        wampsession.call(ak.wwise.core.getInfo, [], {}).then(
            function (res) {
                $('#connectedVersion').text(res.kwargs.version.displayName);
                $('#connectionStatus').text("CONNECTED");
                $('#connectbtn').text("Disconnect");
                $('#connectionStatus').attr('class', 'good');
                //Getting GameParameters objects from Wwise and adding them to the dropdown menus
                var getParamArgs = {
                    from: {ofType: ["GameParameter"]}
                };
                var options = { return: ["name", "id", "@Min", "@Max", "@SimulationValue"]};

                wampsession.call(ak.wwise.core.object.get, [], getParamArgs, options).then(
                    function (objectspayload) {
                        var payload = objectspayload.kwargs;
                        payload.return.sort(compareParams);
                        clearSelects();
                        for (var item of payload.return){
                            gameParams[item.name] = item;

                            $('#xParams').append($('<option/>', {
                                value: item.name,
                                text : item.name
                            }));
                            $('#yParams').append($('<option/>', {
                                value: item.name,
                                text : item.name
                            }));

                            var uri = ak.wwise.core.object.propertyChanged;
                            var options = {
                                object: item['id'],
                                property: 'SimulationValue',
                                return: ["name", "id", "@SimulationValue"]
                            }
                            wampsession.subscribe(uri, updatePosition, options);
                        }
                    },
                    onError
                ); // End of ak.wwise.core.object.get callback
            },
            onError
        ); // End of ak.wwise.core.getInfo callback
    };

}


/**
 * Listener function for drag events on the marker.
 * Updates the position, value label, and sends an update to Wwise
 * @param evt the CreateJS MouseEvent. See http://createjs.com/docs/easeljs/classes/MouseEvent.html
 */
function onDragMove() {
    if (dragging) {
        var newPosition = limit(data.getLocalPosition(this.parent));
        //if(newPosition.x <= renderer.view.width && newPosition.x >= 0 && newPosition.y >= 0 && newPosition.y <= renderer.view.height){
            marker.x = newPosition.x;
            marker.y = newPosition.y;
            // Allows dragging, but send value update if at least one of the parameter is selected
            if(connection.isOpen && (currentXParam || currentYParam)){
                if (marker.x >= renderer.view.width-1) {
                    var xRatio = 1;
                } else if (marker.x <= 1) {
                    var xRatio = 0;
                } else {
                    var xRatio = toRatio(0,renderer.view.width,marker.x);    
                }
                if (marker.y >= renderer.view.height-1) {
                    var yRatio = 0;
                } else if (marker.y <= 1) {
                    var yRatio = 1;
                } else {
                    var yRatio = 1-toRatio(0,renderer.view.height,marker.y);
                }
                updateParamValue(xRatio,yRatio);
            } else {
                $('#xValue').text((newPosition.x/renderer.view.width*100).toFixed(0));
                $('#yValue').text((100-(newPosition.y/renderer.view.height*100)).toFixed(0));
            }
            renderer.render(stage);
       // }
    }
}

function updateParamValue(xRatio, yRatio){
    if(currentXParam){
        var newXVal = (fromRatio(currentXParam['@Min'], currentXParam['@Max'], xRatio)).toFixed(2);
        $('#xValue').text(newXVal);
        sendUpdate(currentXParam['id'], newXVal);
        currentXParam['@SimulationValue'] = newXVal;
    }
    if(currentYParam){
        var newYVal = (fromRatio(currentYParam['@Min'], currentYParam['@Max'], yRatio)).toFixed(2);
        $('#yValue').text(newYVal);
        sendUpdate(currentYParam['id'], newYVal);
        currentYParam['@SimulationValue'] = newYVal;
    }
}

function onDragStart(event) {
    data = event.data;
    dragging = true;
}
function onDragEnd(evt){
    dragging=false;
    data=null;
}

function limit(pos){
    if(pos.x<0){
        pos.x=0
    } else if (pos.x > renderer.view.width) {
        pos.x = renderer.view.width;
    }
    if(pos.y<0){
        pos.y=0
    } else if (pos.y > renderer.view.height) {
        pos.y = renderer.view.height;
    }
    return pos;
}

// Utility function to calculate ratio from absolute value
function toRatio(min, max, value) {
    return (value-min)/(max-min);
}
// Utility function to calculate absolute value from ratio
function fromRatio(min, max, ratio) {
    return (ratio*(max-min))+min;
}

/**
 * Sends an update to Wwise through WAAPI to update the parameter value
 * @param paramId The GUID of the GameParameter to update
 * @param newValue The new value to set
 */
function sendUpdate(paramId, newValue){

    var args = {
        object: paramId,
        property: 'SimulationValue',
        value: parseFloat(newValue)
    }
    wampsession.call(ak.wwise.core.object.setProperty, [], args).then(
        function (res) {},
        onError
    );
}

function connect() {
    if(connection.isOpen) {
        endConnection();
    } else {
        connection.open();
    }
}

function onError(err){
    console.log(`error: ${err}`);
    endConnection();
}


/**
 * Updates the position of the marker and the value labels with the data received from Wwise
 * @param array ArrayPayload received by the topic's publication
 * @param payload ObjectPayload received by the topic's publication
 */
function updatePosition(array, payload){
    
    if(!dragging){
        $.each(gameParams, function(k, v) {
            if (v['id'] === payload['object']['id']){
                v["@SimulationValue"] = payload["new"];
                if(v.name === currentXParam['name']){
                    marker.x = renderer.view.width*toRatio(currentXParam['@Min'],currentXParam['@Max'],payload['new']);
                    $('#xValue').text(payload['new']);
                }
                if (v.name === currentYParam['name']) {
                    marker.y = renderer.view.height*(1-toRatio(currentYParam['@Min'],currentYParam['@Max'],payload['new']));
                    $('#yValue').text(payload['new']);
                }
                renderer.render(stage);
                return false; // exit from loop
            }
        });
    }

}

/**
 * Closing WAMP connection and displaying the right status
 */
function endConnection(){
    connection.close();
    console.log("CLOSING CONNECTION");
    currentXParam = null;
    currentYParam = null;
    $('#connectedVersion').text("--");
    $('#connectionStatus').text("DISCONNECTED");
    $('#connectionStatus').attr('class', 'error');
    $('#connectbtn').text("Connect");
    clearSelects();
}

/**
 * Comparator function to sort the Game Parameters in the list
 */
function compareParams(a,b) {
    if (a.name < b.name)
        return -1;
    if (a.name > b.name)
        return 1;
    return 0;
}

/**
 * Empty Selection boxes and repopulate with default value
 */
function clearSelects(){
    $('#xParams').empty();
    $('#xParams').append($('<option/>', {
        value: "",
        text: "None"
    }));
    $('#yParams').empty();
    $('#yParams').append($('<option/>', {
        value: "",
        text: "None"
    }));
}