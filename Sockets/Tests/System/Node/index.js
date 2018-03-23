//
// Created by TekuConcept on January 1, 2018
// Source: http://codular.com/node-web-sockets
//

console.log("- Beginning Server -");

var http = require('http');
const PORT = 8080;
const MAX_CONNECTIONS = 1;

////////////////////////////////////////////////////////////
//
// Web Server Configuration
//
////////////////////////////////////////////////////////////

var server = http.createServer(function(request, response) {
    response.writeHead(200, {'Content-Type': 'text/html'});
    response.write("Websocket Server");
    response.end();
});

server.listen(PORT, function() {
    console.log((new Date()) + ' Server is listening on port ' + PORT);
});

////////////////////////////////////////////////////////////
//
// WebSocket Server Configuration
//
////////////////////////////////////////////////////////////

var connections = [];
var token = 0;
var WebSocketServer = require('websocket').server;
var wsServer = new WebSocketServer({ httpServer: server });

wsServer.on('request', function(request){
    if(connections.length < MAX_CONNECTIONS) {
        console.log("WebSocket Message: new connection");
        var connection = request.accept(null, request.origin);
        configure(connection);
	}
	else { request.reject(); }
});

var count = 0;
function configure(connection) {
    connection.on('message', function(message) {
        if(message.type === "utf8") {
            console.log("ws: " + message.utf8Data);
            if(message.utf8Data === "ping me") {
                connection.ping();
            }
            else if(message.utf8Data === "ping me data") {
                connection.ping(count+"\n");
            }
            else if(message.utf8Data === "send me data") {
                connection.send("hello from server "+count+"\n");
                count++;
            }
        }
        else {
            var buffer = message.binaryData;
            if(buffer !== undefined && buffer.data !== undefined) {
                for(var i = 0; i < buffer.data.length; i++) {
                    console.log("ws: " + buffer.data[i].toString(16));
                }
            }
        }
        console.log("ws: (message) " + JSON.stringify(message));
    });
    
    connection.on('frame', function(frame) {
        console.log("ws: (frame)");
    });
    
    connection.on('error', function(error) {
        console.log("ws: (error) " + error);
    });
    
    connection.on('ping', function(cancle,data) {
        console.log("ws: (ping) " + data);
    });
    
    connection.on('pong', function(data) {
        console.log("ws: (pong) " + data);
    });
    
    connection.on('close', function(reason, data) {
	    console.log("ws: (close) " + reason + "|" + data);
        
        var index = connections.indexOf(connection);
        if(index > -1) connections.splice(index, 1);
    });
}