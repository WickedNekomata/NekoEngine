var wampsession = null, connection=null;
var isConnected=false;
var currentTarget={};
var transports = {};
var mPlayableObj = {}; // local storing of fetched playable objects
const state_disabled = "dis", state_On = "on", state_ready = "nor";
var cmdStateMap = {
    stop : {
        state: state_disabled
    },
    pause : {
        state: state_disabled
    },
    play : {
        state: state_disabled
    }
};

function init() {
    $('#selector').selectpicker({showSubtext: true});
    $('#selector-wrapper').hide();
    $('#selector').on('hidden.bs.select', function(e) {
        var that = $(this);
        if(that.val()){
            // Create transport for this object.
            wampsession.call(ak.wwise.core.transport.getList, [], {}).then(
                function (getRes) {
                    getRes = getRes.kwargs['list'];
                    for (var elem in getRes){
                        transports[elem['object']] = elem;
                    }
                    console.log(that.val());
                    setTarget(that.val());
                },
                onError
            ); // end of ak.wwise.core.transport.getList callback
            
            // TODO Subscribe to this obj transport Events once implemented
        }
    });


    $('.selectableObj').on('click', function(e){
        console.log(this);
    });

    // Trigger search on "ENTER" in search box or search btn
    $('#searchBar').on('keypress', function(e){
        if( e.which === 13){
            performSearch();
        }
    });
    $('#objSearchBtn').on('click', performSearch);


    // Create the WAMP connection
    connection = new autobahn.Connection({
        url: 'ws://localhost:8080/waapi',
        realm: 'realm1',
        protocols: ['wamp.2.json']
    });
    connection.onopen = function(session) {
        wampsession = session;
        // Calling getWwiseInfo in order to get the current Wwise version through WAAPI
        wampsession.call(ak.wwise.core.getInfo, [], {}).then(
            function (res) {
                // testing connection by getting Wwise info
                $('#connectedVersion').text(res.kwargs.version.displayName);
                $('#connectionStatus').text("CONNECTED");
                $('#connectbtn').text("Disconnect");
                $('#connectionStatus').attr('class', 'good');
                isConnected = true;
            },
            onError
        );
    };
}

function setTarget(objID){
    targetTransport = transports[objID]
    if(targetTransport){
        currentTarget['transportID'] = targetTransport[transport];
        setCtrlReady('play', 'pause', 'stop');
    } else {
        var createArgs = {object: objID};
        wampsession.call(ak.wwise.core.transport.create, [], createArgs).then(
            function (createRes) {
                currentTarget['transportID'] = createRes.kwargs['transport'];
                setCtrlReady('play', 'pause', 'stop');
            },
            onError
        ); // end of ak.wwise.core.transport.create callback
    }
}

function performSearch(){
    var queryStr = $('#searchBar').val();
    if(isConnected){
        var getObjArgs = {
            from: { search: [queryStr]}            
        };
        var options = {return: ["name", "id", "type", "isPlayable"]};
        // Retrieve the list of objects whose name contain the queryStr
        wampsession.call(ak.wwise.core.object.get, [], getObjArgs, options).then(
            function (objectspayload) {
                var payload = objectspayload.kwargs.return.sort(comparator);
                clearSelect();
                // Add the received objects to the dropdown selector
                for (var i in payload){
                    if (payload[i].isPlayable) {
                        mPlayableObj[payload[i].id] = payload[i];
                        $('#selector').append($('<option/>', {
                            value: payload[i].id,
                            text : payload[i].name,
                            class: "selectableObj",
                            "data-subtext" : " ["+payload[i].type+"]" // bootstrap-select specific for subtext
                        }));
                    }
                }
                // 
                $('#selector-wrapper').show();
                $('#selector').selectpicker('refresh');
                if($('#selector').val()){
                    setTarget($('#selector').val());
                } else {
                    setCtrlDisabled('play', 'pause', 'stop');
                }
            },
            onError
        ); // End of ak.wwise.core.object.get callback
    }
}

/**
 * Connection Manager
 */
function connect() {
    if(connection.isOpen && isConnected) {
        endConnection();
    } else {
        connection.open();
        $('#selector').prop("disabled", false);
    }
}

/**
 * Generic error handling
 */
function onError(err){
    console.log(err);
    console.log(`error: ${err}`);
    endConnection();
}

/*
*   Utils UI functions
*/ 
function hover(element, cmd) {
    if(cmdStateMap[cmd].state !== state_disabled)
        element.setAttribute('src', `assets/${cmd}_${cmdStateMap[cmd].state}_hot.PNG`);
}
function unhover(element, cmd) {
    if(cmdStateMap[cmd].state !== state_disabled)
        element.setAttribute('src', `assets/${cmd}_${cmdStateMap[cmd].state}.PNG`);
}
function cmdclick(element, cmd) {
    if(cmdStateMap[cmd].state !== state_disabled) {
        element.setAttribute('src', `assets/${cmd}_pre.PNG`);
        launch(cmd);
    }
}
function cmdunclick(element, cmd) {
    if(cmdStateMap[cmd].state !== state_disabled) 
        element.setAttribute('src', `assets/${cmd}_${cmdStateMap[cmd].state}.PNG`);
}
function launch(cmd) {
    //cmdStateMap[cmd].state=state_On;
    $('#'+cmd).attr('src', `assets/${cmd}_on.PNG`);
    var args = {
        transport: currentTarget['transportID'],
        action: cmd
    }
    wampsession.call(ak.wwise.core.transport.executeAction, [], args).then(
        function() {},
        onError
    );
}
function setCtrlReady(...controls){
    for (var i in controls){
        cmdStateMap[controls[i]].state = state_ready;
        $('#'+controls[i]).attr('src', `assets/${controls[i]}_nor.PNG`);
    }
}
function setCtrlDisabled(...controls){
    for (var i in controls){
        cmdStateMap[controls[i]].state = state_disabled;
        $('#'+controls[i]).attr('src', `assets/${controls[i]}_nor_dis.PNG`);
    }
}

/**
 * Closing WAMP connection and displaying the right status
 */
function endConnection(){
    connection.close();
    isConnected = false;
    console.log("CLOSING CONNECTION");
    $('#connectedVersion').text("--");
    $('#connectionStatus').text("DISCONNECTED");
    $('#connectionStatus').attr('class', 'error');
    $('#connectbtn').text("Connect");
    $('#selector').prop("disabled", true);
    $('#selector-wrapper').hide();
    setCtrlDisabled('play','stop','pause');
}

/**
 * Comparator function to sort the Objects/Events in the list
 */
function comparator(a,b) {
    if (a.name < b.name)
        return -1;
    if (a.name > b.name)
        return 1;
    return 0;
}

/**
 * Empty selector box and repopulate with default value
 */
function clearSelect(){
    for(var obj in mPlayableObj){
        $('#selector').find("[value='"+obj+"']").remove();
    }
    mPlayableObj = {};
    $('#selector').selectpicker('refresh');
}