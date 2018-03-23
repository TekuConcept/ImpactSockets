//
// Created by TekuConcept on January 1, 2018
// Source: http://codular.com/node-web-sockets
//

var http = require('http');
var fs   = require("fs");
const PORT = 8080;
const MAX_CONNECTIONS = 1;

////////////////////////////////////////////////////////////
//
// Web Server Configuration
//
////////////////////////////////////////////////////////////

var server = http.createServer(function(request, response) {
    if(request.url === "/"){
        fs.readFile("index.html", function (err, data) {
            if(err) {
                console.error(err.message);
                process.exit(1);
            }
            response.writeHead(200, {'Content-Type': 'text/html'});
            response.write(data);
            response.end();
        });
    }
    else{
        response.writeHead(200, {'Content-Type': 'text/html'});
        response.write('No path to URL ' + request.url);
        response.end();
    }
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
        configureWSMessage(connection);
        configureWSClose(connection);
        var status_msg = "Hello Client";
        connection.send(JSON.stringify({message:status_msg}));
        connections.push(connection);
	}
	else {
		request.reject();
	}
});

function configureWSMessage(connection) {
    connection.on('message', function(message) {
        if(message.type === "utf8") {
            try{
                console.log(message.utf8Data);
                token++;
                connection.send(
                    JSON.stringify({ message: "Hello Client " + token }));
            } catch(e) {
                console.log("WebSocket Message: " + e.message);
            }
        }
        
        console.log("WebSocket Message: " + JSON.stringify(message));
    });
}

function configureWSClose(connection) {
    connection.on('close', function(conn) {
	    console.log("WebSocket Message: closed");
        
        var index = connections.indexOf(connection);
        if(index > -1)
            connections.splice(index, 1);
    });
}