var CMD = {
	QUIT:		1,
	READY:		2,
	WINDOW:		3,
	GET_BIKES:	100,
	GET_IMAGE:  101,
};

var NETIMAGE_CMD = {
    BEGIN: 1,
    DATA:  2,
   	END:   3
};

var WINDOW = {
	MAIN_MENU: 0,
	BIKE_LIST: 1
};

var apiBase = "http://tfl.jonasbergler.com";
var DB = {};
DB.location = {"speed":null,"heading":null,"altitudeAccuracy":null,"accuracy":27,"altitude":null,"longitude":-0.132521,"latitude":51.522549};

Pebble.addEventListener("ready", function(e) {
	console.log("#$# ready => " + e.ready);
	//fetchBikeInfo(DB.location);	
});

Pebble.addEventListener("appmessage", function(e) {
	console.log("Message from device: " + JSON.stringify(e.payload));
	var cmd = null;

	if (typeof e.payload.cmd !== "undefined") cmd = e.payload.cmd;
	else if (typeof e.payload.netimageCmd !== "undefined") cmd = e.payload.netimageCmd;
	else return;

	switch (cmd) {
		case CMD.READY:
			console.log("Watch ready.");
			Pebble.sendAppMessage({ 'cmd': CMD.READY });
			break;

		case CMD.WINDOW:
			console.log("Window " + e.payload.window + " selected");
			Pebble.sendAppMessage({ 'cmd': CMD.WINDOW, 'window': e.payload.window });
			break;

		case CMD.GET_BIKES:
			setTimeout(fetchAndSendBikeInfo, 100);
			break;

		case NETIMAGE_CMD.BEGIN:
			console.log("netimage begin");
			setTimeout(function() { fetchImage(e.payload.netimageUrl, e.payload.netimageChunkSize, DB.location); }, 200);
	}
});
console.log("Pebble.addEventListener() calls complete.");

function registerWebapp(e) {
	console.log("registerWebapp()");
	console.log(JSON.stringify(this));
	return false;
}

function fetchAndSendBikeInfo() {
	console.log("fetchAndSendBikeInfo() called.");
	//We already have the data, so just send it.
	if (DB.bikes && DB.bikes.length) {
		sendBikeInfo();
	}
	else {
		fetchBikeInfo(DB.location, sendBikeInfo, function() { console.log("fetchBikeInfo() failed"); });
	}
}
function sendBikeInfo(successCb, failureCb) {
	var retries = 0;

	success = function() { if (successCb != undefined) successCb(); };
	failure = function(e) { if (failureCb != undefined) failureCb(e); };

	sendBike = function(bike) {
		console.log("Sending station #" + bike.bikeId);
		bike.cmd = CMD.GET_BIKES;
		Pebble.sendAppMessage(
			bike,
			function(e) {
				// Successfully sent the last bike, send the next?
				if (DB.bikes.length) {
					sendBike(DB.bikes.shift());
				}
				else {
			 		Pebble.sendAppMessage({'cmd': CMD.GET_BIKES, 'bikeId': 255}, success, failure);
				}
			},
			function (e) {
				// Send failed, try again?
				if (retries++ < 3) {
			  		console.log("AppMessage[NACK] for station #" + bike.bikeId + " - Retry...");
			  		sendBike(bike);
				}
				else {
			  		console.log("AppMessage[NACK] for station #" + bike.bikeId + " - Giving up...");
			  		failure(e);
				}
			}
		);
	};

	sendBike(DB.bikes.shift());
}

function fetchBikeInfo(coords, successCb, failureCb) {
	console.log("fetchBikeInfo() called");

	var retries = 0;
	var result = false;

	success = function() { if (successCb != undefined) successCb(); };
	failure = function(e) { if (failureCb != undefined) failureCb(e); };

	var lat = coords.latitude;
	var lng = coords.longitude;

	var response;
	var url = apiBase + "/bikes.php?lat=+" + lat + "&lng=" + lng;
	var req = null;

	handleData = function(e) {
		if (!req) {
			failure();
			return;
		}
		if (req.readyState == 4) {
			console.log("AJAX Response[" + req.status + "/" + url + "]");
			if(req.status == 200) {
				data = JSON.parse(req.responseText);
				if (data && data.length > 0) {
					DB.bikes = [];
					for(i = 0; i < data.length; i++) {
						var bike = {
				            "bikeName": data[i].name,
				            "bikeInfo": data[i].distance + "m " + data[i].bearing + ", " + data[i].availableBikes + " (" + data[i].availableDocks + ")",
				            "bikeUrl": "http://tfl.jonasbergler.com/bikeMap.php?stationId=" + data[i].id + "%%COORDS%%",
				            "bikeId": i
				        }
				        DB.bikes[i] = bike;
					}
					result = true; 
					success();
				}
			} else {
				failure();
			}
		}
	};
	handleTimeout = function() {
		console.log("AJAX Timeout[" + url + "]");
		if (retries++ < 3) {
			getData();
		}
		else {
			failure();
		}
	}
	getData = function() {
		req = new XMLHttpRequest();
		req.open('GET', url, true);
		req.onload = handleData;
		req.timeout = 3000;
    	req.ontimeout = handleTimeout;
		req.send(null);
		console.log("Sent request to " + url);
	}
	getData();
}

function fetchImage(reqUrl, chunkSize, coords) {
	var lat = coords.latitude;
	var lng = coords.longitude;

	var response;
	var coords = "&lat=" + lat + "&lng=" + lng;
	var url = reqUrl.replace("%%COORDS%%", coords);
	var req = null;

	handleData = function(e) {
		if (!req) {
			failure();
			return;
		}
		if (req.readyState == 4) {
			console.log("AJAX Response[" + req.status + "/" + url + "]");
			if(req.status == 200) {
				image = (req.responseText);
				sendImage(image, chunkSize);
			} else {
				failure();
			}
		}
	};
	handleTimeout = function() {
		console.log("AJAX Timeout[" + url + "]");
		if (retries++ < 3) {
			getData();
		}
		else {
			failure();
		}
	}
	getData = function() {
		req = new XMLHttpRequest();
		req.open('GET', url, true);
		req.onload = handleData;
		req.timeout = 3000;
    	req.ontimeout = handleTimeout;
		req.send(null);
		console.log("Sent request to " + url);
	}
	getData();
}

function sendImage(img, chunkSize, successCb, failureCb) {
	if (!chunkSize) chunkSize = 512;
	if (chunkSize < 32 || chunkSize > 512) chunkSize = 128;

	success = function() { if (successCb != undefined) successCb(); };
	failure = function(e) { if (failureCb != undefined) failureCb(e); };

	sendChunk = function(start) {
		var dataToSend = img.slice(start, start+chunkSize);

		console.log("Sending chunk '" + dataToSend + "'");
		msg = {'netimageCmd': NETIMAGE_CMD.DATA, 'netimageData': dataToSend };

		Pebble.sendAppMessage(
			msg,
			function(e) {
				// Successfully sent the last bike, send the next?
				if (start + chunkSize < img.length) {
					sendChunk(start + chunkSize);
				}
				else {
			 		Pebble.sendAppMessage({'netimageCmd': NETIMAGE_CMD.END}, success, failure);
				}
			},
			function (e) {
				// Send failed, try again?
				if (retries++ < 3) {
			  		console.log("AppMessage[NACK] for chunk - Retry...");
			  		sendChunk(start);
				}
				else {
			  		console.log("AppMessage[NACK] for chunk - Giving up...");
			  		failure(e);
				}
			}
		);
	};

	start =	{'netimageCmd': NETIMAGE_CMD.BEGIN, 'netimageSize': img.length };
	console.log("sending begin: " + JSON.stringify(start));
	Pebble.sendAppMessage(start, function() {
		sendChunk(0);
	});
}

function trace() {
	var e = new Error('dummy');
	var stack = e.stack.replace(/^[^\(]+?[\n$]/gm, '')
		.replace(/^\s+at\s+/gm, '')
		.replace(/^Object.\s*\(/gm, '{anonymous}()@');
	console.log(stack);
}
