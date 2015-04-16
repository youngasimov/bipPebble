var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var KEY_BIP = 0;

function getSaldo(bip){
	var url = 'http://saldobip.dreamer8.com/api/index.php/saldo/';
	// Send request to OpenWeatherMap
	xhrRequest(url+bip, 'GET', function(responseText) {
		var json = JSON.parse(responseText);
		var dictionary = {};
		if(json.saldo !== undefined){
			dictionary.KEY_ERROR = 0;
			dictionary.KEY_SALDO = json.saldo;
		}else if(json.error !== undefined){
			dictionary.KEY_ERROR = 2;
			dictionary.KEY_SALDO = 0;
		}else{
			dictionary.KEY_ERROR = 3;
			dictionary.KEY_SALDO = 0;
		}
		Pebble.sendAppMessage(dictionary,function(e) {
			console.log('Balance info sent to Pebble successfully!');
		}, function(e) {
			console.log('Error sending balance info to Pebble!');
		});
	});
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
	var value = localStorage.getItem(KEY_BIP);
	if(value === null){
		var dictionary = {
			'KEY_ERROR': 1,
			'KEY_SALDO': 0
		};
		Pebble.sendAppMessage(dictionary,function(e) {
			console.log('Balance info sent to Pebble successfully!');
		}, function(e) {
			console.log('Error sending balance info to Pebble!');
		});
	}else{
		getSaldo(value);	
	}
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
var value = localStorage.getItem(KEY_BIP);
	if(value === null){
		var dictionary = {
			'KEY_ERROR': 1,
			'KEY_SALDO': 0
		};
		Pebble.sendAppMessage(dictionary,function(e) {
			console.log('Balance info sent to Pebble successfully!');
		}, function(e) {
			console.log('Error sending balance info to Pebble!');
		});
	}else{
		getSaldo(value);	
	}
});

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
	var url = 'http://saldobip.dreamer8.com/pebble/index.html';
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed',
  function(e) {
		console.log(e.response);
    var config = JSON.parse(decodeURIComponent(e.response));
		localStorage.setItem(KEY_BIP,config.saldo);
		getSaldo(config.saldo);
  }
);