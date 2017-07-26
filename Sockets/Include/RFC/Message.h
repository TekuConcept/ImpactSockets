/**
 * 
 */

#ifndef RFC_MESSAGE_H
#define RFC_MESSAGE_H

namespace Impact {
namespace RFC2616 {
    class Message {
    public:
        virtual bool parse(std::istream &request) = 0;
    };
}}

#endif