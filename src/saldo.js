var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
		var url = 'http://default-environment-mn23kbgwjm.elasticbeanstalk.com/api/index.php/saldo/82728479';
		// Send request to OpenWeatherMap
		xhrRequest(url, 'GET', function(responseText) {
			var json = JSON.parse(responseText);
      console.log('JSON is ' + json);
    }      
  );
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);