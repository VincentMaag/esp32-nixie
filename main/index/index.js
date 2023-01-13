/*
    Webpage for Nixie Clock

*/


// =========================================================================================
function init(){
    console.log("Init called");

    // Hook events to buttons
    // document.getElementById("buttonSingleColor").addEventListener("click", changeToSingleColor);
    

    
    
}
// =========================================================================================

function sendSomething(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            // do nothing with response
            ;
        }
    };
    // build string to send (POST)
    var postCommandString = "sendGRB" + grb;

    xhttp.open("POST", postCommandString, true);
    xhttp.responseType = "arraybuffer";
    xhttp.send();
}

var myArrayResponse = 0;
//var bla = setInterval(getSomething, 1000);
function getSomething(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {

            console.log("Got an answer");
            var myArrayResponse = new Uint8Array(this.response);
            console.log(myArrayResponse);
        }
    };
    console.log("Requesting Something");
    xhttp.open("GET", "getSomething", true);
    xhttp.responseType = "arraybuffer";
    xhttp.send();  
}


var myArrayResponse = 0;

var bla = setInterval(getEspTime, 1000);

function getEspTime(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {

            console.log("Got an answer");
            var myArrayResponse = this.response;
            console.log(myArrayResponse);

            document.getElementById("txtTimeDisplay").innerHTML = myArrayResponse;

        }
    };
    xhttp.open("GET", "getEspTime", true);
    xhttp.responseType = "text";
    xhttp.send();  
}





