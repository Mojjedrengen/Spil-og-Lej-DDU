//const mqtt = require("mqtt");
let client = mqtt.connect("wss://mqtt.nextservices.dk");

let connectionDiv

//var start;
var state = 0;


function setup(){
    document.getElementById('timer').addEventListener('click', start);
    connectionDiv = select('#connection');

    client = mqtt.connect('wss://mqtt.nextservices.dk');

    client.on('connect', (m) => {
        console.log('Client connected: ', m)
        connectionDiv.html('You are now connected to mqtt.nextservices.dk')
      });

    client.subscribe('DDU4/FAMS/SpilOgLej');

    client.on('message', (topic, message) => {
        if (message == "MorseKodeTheKeyIsKeyIsDone") {
            document.getElementById("morse").innerHTML = "Gåden er løst";
        } else if (message == "SetThetempurturIsDone") {
            document.getElementById('temperatur').innerHTML = "Gåden er løst";
        } else if (message == 'RGBKodeIsDone'){
            document.getElementById('rgbkode').innerHTML = "Gåden er løst";
        } else if (message == 'SortHvidKodeIsDone'){
            document.getElementById('sorthvid').innerHTML = "Gåden er løst";
        }
        

    });
}

function start() {
    state = 1;

    client.publish("DDU4/FAMS/SpilOgLej", "start");
}
