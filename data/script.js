var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

var coordinates;
var map;
var marker;

var canSend = true; // Flag to control when it's okay to send a message

function onload(event) {
    initWebSocket();
    initMap(); 
}

function getBusInfo() {
    if (websocket.readyState === WebSocket.OPEN && canSend) {
        websocket.send("getBusInfo");
        canSend = false; // Set the flag to false until an acknowledgement is received
    } else {
        console.log('Waiting to send new message');
    }
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    setInterval(getBusInfo, 3000);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

const x = document.getElementById("demo");

function getLocation() {
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(showPosition);
    } else {
        x.innerHTML = "Geolocation is not supported by this browser.";
    }
}
function showPosition(position) {
    x.innerHTML = "Latitude: " + position.coords.latitude +
        "<br>Longitude: " + position.coords.longitude;
}

// Initialize and add the map
function initMap() {
    // Specify the coordinates (X, Y)
    coordinates = [10.772067412563548, 106.65784201489897]; // Coordinates

    // Create a new map centered at the specified coordinates
    map = L.map('map').setView(coordinates, 15);

    // Add the base map layer
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    // Create a marker positioned at the specified coordinates and add it to the map
    marker = L.marker(coordinates).addTo(map)
    .bindPopup('Bé Ling ngáo đét :3')
    .openPopup();
}

// function onMessage(event) {
//     console.log(event.data);
//     var myObj = JSON.parse(event.data);
//     var keys = Object.keys(myObj);

//     for (var i = 0; i < keys.length; i++){
//         var key = keys[i];
//         document.getElementById(key).innerHTML = myObj[key];
//     }
//     coordinates[0] = Number(myObj[keys[2]]);
//     coordinates[1] = Number(myObj[keys[3]]);
//     marker.setLatLng(coordinates);
// }

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        document.getElementById(key).innerHTML = myObj[key];
    }

    coordinates[0] = Number(myObj[keys[2]]);
    coordinates[1] = Number(myObj[keys[3]]);
    marker.setLatLng(coordinates);

    // if (myObj.type === "acknowledgement") { // Assuming server sends this type of message as an ack
        canSend = true; // Allow new messages to be sent
    // }
}