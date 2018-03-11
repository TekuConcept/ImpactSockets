/**
 * Created by TekuConcept on March 8, 2018
 */

#include "RFC/wsmanip.h"
#include "Websocket.h"

using namespace Impact;

/**
 * The try-catch casting seems a little hackish but it does the job nicely.
 */

std::ostream& ws::text(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.out_mode(WS_MODE::TEXT);
    } catch(...) { /* not a websocket */ }
    return stream;
}

std::ostream& ws::binary(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.out_mode(WS_MODE::BINARY);
    } catch(...) { /* not a websocket */ }
    return stream;
}

std::ostream& ws::ping(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.ping();
    } catch(...) { /* not a websocket */ }
    return stream;
}

std::ostream& ws::close(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.close();
    } catch(...) { /* not a websocket */ }
    return stream;
}

std::ostream& ws::send(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.send();
    } catch(...) { /* not a websocket */ }
    return stream;
}

std::ostream& ws::push(std::ostream& stream) {
    try {
        Websocket& socket = dynamic_cast<Websocket&>(stream);
        socket.push();
    } catch(...) { /* not a websocket */ }
    return stream;
}