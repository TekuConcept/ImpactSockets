/**
 * Created by TekuConcept on January 18, 2021
 */

#ifndef IMPACT_TCP_CLIENT_H
#define IMPACT_TCP_CLIENT_H

#include <string>
#include <functional>
#include <memory>
#include "async/event_loop_interface.h"
#include "sockets/types.h"
#include "sockets/tcp_client_interface.h"

namespace impact {

    // FIXME: forward events from m_base to this
    class tcp_client : public tcp_client_interface {
    public:
        tcp_client(std::shared_ptr<event_loop_interface>
            event_loop = default_event_loop());
        ~tcp_client() = default;

        tcp_address_t address() const override;
        size_t bytes_read() const override;
        size_t bytes_written() const override;
        bool connecting() const override;
        bool destroyed() const override;
        std::string local_address() const override;
        unsigned short local_port() const override;
        bool pending() const override;
        std::string remote_address() const override;
        address_family remote_family() const override;
        unsigned short remote_port() const override;
        size_t timeout() const override;
        std::string ready_state() const override;

        tcp_client_interface* connect(
            std::string path,
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* connect(
            unsigned short port,
            event_emitter::callback_t cb) override;
        tcp_client_interface* connect(
            unsigned short port,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* destroy(std::string error = "") override;
        tcp_client_interface* end(event_emitter::callback_t cb) override;
        tcp_client_interface* end(
            std::string data,
            event_emitter::callback_t cb) override;
        tcp_client_interface* end(
            std::string data = std::string(),
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* pause() override;
        tcp_client_interface* resume() override;
        tcp_client_interface* set_encoding(
            std::string encoding = "utf8") override;
        tcp_client_interface* set_keep_alive(
            unsigned int initial_delay) override;
        tcp_client_interface* set_keep_alive(
            bool enable = false,
            unsigned int initial_delay = 0) override;
        tcp_client_interface* set_no_delay(
            bool no_delay = true) override;
        tcp_client_interface* set_timeout(
            unsigned int timeout,
            event_emitter::callback_t cb = nullptr) override;
        bool write(
            std::string data,
            event_emitter::callback_t cb = nullptr) override;
        bool write(
            std::string data,
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) override;

        // on('close', (hadError: bool) => void)
        // on('connect', () => void)
        // on('data', (data: string) => void)
        // on('drain', () => void)
        // on('end', () => void)
        // on('error', (error: Error) => void)
        // on('lookup', (err?: Error, address: string, family: address_family, host: string) => void)
        // on('ready', () => void)
        // on('timeout', () => void)

    private:
        std::shared_ptr<tcp_client_interface> m_base;
    };

} /* namespace impact */

#endif /* IMPACT_TCP_CLIENT_H */
