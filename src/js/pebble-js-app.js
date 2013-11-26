var apiBase = "http://tfl.jonasbergler.com";
var DB = {};
DB.location = {"speed":null,"heading":null,"altitudeAccuracy":null,"accuracy":27,"altitude":null,"longitude":-0.141718,"latitude":51.517633};

Pebble.addEventListener("ready", function(e) {
	console.log("#$# ready => " + e.ready);
	fetchBikeInfo(DB.location);	
});

Pebble.addEventListener("appmessage", function(e) {
	console.log("Message from device");
	console.log("e => " + JSON.stringify(e));

	if (e.payload.cmdFetch) {
		if (DB.bikes && DB.bikes.length > 0) {
			if (e.payload.bikeId != "undefined" && e.payload.bikeId < DB.bikes.length ) {
				Pebble.sendAppMessage(DB.bikes[e.payload.bikeId]);
				console.log("CMD=>BIKES MSG=>" + JSON.stringify(DB.bikes[e.payload.bikeId]));
			}
			else
			{
				console.log("CMD=>BIKES but we don't have info on bikeId " + e.payload.bikeId);
			}
		}
	}
});


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
			console.log("AJAX: " + req.status);
			if(req.status == 200) {
				console.log(req.responseText);
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
					Pebble.sendAppMessage(DB.bikes[0]);
					console.log("CMD=>BIKES MSG=>" + JSON.stringify(DB.bikes[0]));
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
