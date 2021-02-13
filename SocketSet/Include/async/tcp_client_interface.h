/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_TCP_CLIENT_INTERFACE_H
#define IMPACT_TCP_CLIENT_INTERFACE_H

#include <string>
#include "sockets/types.h"
#include "utils/event_emitter.h"

namespace impact {

    struct tcp_address_t {
        unsigned short port;
        address_family family;
        std::string address;
    };

    class tcp_client_interface : public event_emitter {
    public:
        virtual ~tcp_client_interface() = default;

        virtual tcp_address_t address() const = 0;
        virtual size_t bytes_read() const = 0;
        virtual size_t bytes_written() const = 0;
        virtual bool connecting() const = 0;
        virtual bool destroyed() const = 0;
        virtual std::string local_address() const = 0;
        virtual unsigned short local_port() const = 0;
        virtual bool pending() const = 0;
        virtual std::string remote_address() const = 0;
        virtual address_family remote_family() const = 0;
        virtual unsigned short remote_port() const = 0;
        virtual size_t timeout() const = 0;
        virtual std::string ready_state() const = 0;

        virtual tcp_client_interface* connect(
            std::string path,
            event_emitter::callback_t cb/* = nullptr*/) = 0;
        virtual tcp_client_interface* connect(
            unsigned short port,
            event_emitter::callback_t cb) = 0;
        virtual tcp_client_interface* connect(
            unsigned short port,
            std::string host/* = 127.0.0.1*/,
            event_emitter::callback_t cb/* = nullptr*/) = 0;
        virtual tcp_client_interface* destroy(std::string error = "") = 0;
        virtual tcp_client_interface* end(event_emitter::callback_t cb) = 0;
        virtual tcp_client_interface* end(
            std::string data,
            event_emitter::callback_t cb) = 0;
        virtual tcp_client_interface* end(
            std::string data/* = ""*/,
            std::string encoding/* = "utf8"*/,
            event_emitter::callback_t cb/* = nullptr*/) = 0;
        virtual tcp_client_interface* pause() = 0;
        virtual tcp_client_interface* resume() = 0;
        virtual tcp_client_interface* set_encoding(
            std::string encoding/* = "utf8"*/) = 0;
        virtual tcp_client_interface* set_keep_alive(
            unsigned int initial_delay) = 0;
        virtual tcp_client_interface* set_keep_alive(
            bool enable/* = false*/,
            unsigned int initial_delay/* = 0*/) = 0;
        virtual tcp_client_interface* set_no_delay(
            bool no_delay/* = true*/) = 0;
        virtual tcp_client_interface* set_timeout(
            unsigned int timeout,
            event_emitter::callback_t cb/* = nullptr*/) = 0;
        virtual bool write(
            std::string data,
            event_emitter::callback_t cb/* = nullptr*/) = 0;
        virtual bool write(
            std::string data,
            std::string encoding/* = "utf8"*/,
            event_emitter::callback_t cb/* = nullptr*/) = 0;

        // on('close', (hadError: bool) => void)
        // on('connect', () => void)
        // on('data', (data: string) => void)
        // on('drain', () => void)
        // on('end', () => void)
        // on('error', (error: Error) => void)
        // on('lookup', (err?: Error, address: string, family: address_family, host: string) => void)
        // on('ready', () => void)
        // on('timeout', () => void)
    };

} /* namespace impact */

#endif /* IMPACT_TCP_CLIENT_INTERFACE_H */
