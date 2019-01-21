console.log("app.js loaded.");

var socket = io('ws://localhost:8081');
socket.on('connect', function() {
	socket.send('hallo welt');

	socket.on('message', function(data) {
		console.log(data);
	});

});
