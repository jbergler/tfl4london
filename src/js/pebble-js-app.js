var CMD = {
	QUIT:		1,
	READY:		2,
	WINDOW:		3,
	GET_BIKES:	100
};

var WINDOW = {
	MAIN_MENU: 0,
	BIKE_LIST: 1
};

var apiBase = "http://tfl.jonasbergler.com";
var DB = {};
DB.location = {"speed":null,"heading":null,"altitudeAccuracy":null,"accuracy":27,"altitude":null,"longitude":-0.141718,"latitude":51.517633};

Pebble.addEventListener("ready", function(e) {
	console.log("#$# ready => " + e.ready);
	fetchBikeInfo(DB.location);	
});

Pebble.addEventListener("appmessage", function(e) {
	console.log("Message from device: " + JSON.stringify(e.payload));

	switch (e.payload.cmd) {
		case CMD.READY:
			console.log("Watch ready.");
			Pebble.sendAppMessage({ 'cmd': CMD.READY });
			break;

		case CMD.WINDOW:
			console.log("Window " + e.payload.window + " selected");
			Pebble.sendAppMessage({ 'cmd': CMD.WINDOW, 'window': e.payload.window });
			break;

		case CMD.GET_BIKES:
			var bike = DB.bikes[e.payload.bikeId];
			bike.cmd = CMD.GET_BIKES;
			console.log("Sent " + JSON.stringify(bike));
			console.log("CMD=>GET_BIKES BIKE_ID=>" + e.payload.bikeId);
			Pebble.sendAppMessage(bike);
			break;
	}
});
console.log("Pebble.addEventListener() calls complete.");

function registerWebapp(e) {
	console.log("registerWebapp()");
	console.log(JSON.stringify(this));
	return false;
}

function fetchBikeInfo(coords) {
	var lat = coords.latitude;
	var lng = coords.longitude;
	console.log("LOC=>" + JSON.stringify(coords));

	var response;
	var url = apiBase + "/bikes.php?lat=+" + lat + "&lng=" + lng;
	var req = new XMLHttpRequest();
	req.open('GET', url, true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			console.log("AJAX Response[" + req.status + "/" + url + "]: " + req.responseText);
			if(req.status == 200) {
				data = JSON.parse(req.responseText);
				if (data && data.length > 0) {
					DB.bikes = [];
					for(i = 0; i < data.length; i++) {
						var msg = {
				            "bikeName": data[i].station.name,
				            "bikeInfo":data[i].distance + "m away, " + data[i].station.nbBikes + " bikes available.",
				            "bikeId": i
				        }
				        DB.bikes[i] = msg;
					}
				}
				// var temperature, icon, city;
				// if (response && response.list && response.list.length > 0) {
				// 	var weatherResult = response.list[0];
				// 	temperature = Math.round(weatherResult.main.temp - 273.15);
				// 	icon = iconFromWeatherId(weatherResult.weather[0].id);
				// 	city = weatherResult.name;
				// 	console.log(temperature);
				// 	console.log(icon);
				// 	console.log(city);
				// 	Pebble.sendAppMessage({
				// 	"icon":icon,
				// 	"temperature":temperature + "\u00B0C",
				// 	"city":city});
				// }

			} else {
				console.log("Error");
			}
		}
	}
	req.send(null);
	console.log("Sent request to " + url);
}
