var showMessage = function(message){
  document.getElementById("message").innerHTML = message; 
}

function onBodyLoad()
{
    var request = new XMLHttpRequest();

    request.onload = function () {
       var res = JSON.parse(request.responseText);
       showMessage(`Hello ${res.displayName} ${res.version.displayName}!`);
    };
    
    request.open("POST", "http://localhost:8090/waapi", true);
    
    request.setRequestHeader("Content-Type", "application/json");
    
    var data = {
        uri: ak.wwise.core.getInfo,
        options: {},
        args: {}
    };
    
    request.send(JSON.stringify(data));
}
