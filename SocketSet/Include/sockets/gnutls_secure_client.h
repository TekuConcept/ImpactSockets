/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_SOCKETS_GNUTLS_SECURE_CLIENT_H
#define IMPACT_SOCKETS_GNUTLS_SECURE_CLIENT_H

#include <memory>
#include <string>
#include <mutex>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "interfaces/secure_client_interface.h"
#include "interfaces/tcp_client_interface.h"
#include "interfaces/event_loop_interface.h"

namespace impact {

    class gnutls_secure_client :
        public secure_client_interface,
        protected tcp_client_observer_interface
    {
    public:
        gnutls_secure_client(tcp_client_t base =
            default_event_loop()->create_tcp_client());
        ~gnutls_secure_client() = default;

        //
        // -- secure_client_interface --
        //

        const std::string& server_name() const override;
        void server_name(std::string host) override;
        bool cert_verify_enabled() const override;
        void cert_verify_enabled(bool enabled) override;

        void set_x509_credentials(
            std::string key, std::string certificate) override;

        //
        // -- tcp_client_interface --
        //

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
        ready_state_t ready_state() const override;

        tcp_client_interface* connect(
            std::string path,
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* connect(
            unsigned short port,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* destroy(std::string error = "") override;
        tcp_client_interface* end(
            std::string data = std::string(),
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) override;
        tcp_client_interface* pause() override;
        tcp_client_interface* resume() override;
        tcp_client_interface* set_encoding(
            std::string encoding = "utf8") override;
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
            std::string encoding = "utf8",
            event_emitter::callback_t cb = nullptr) override;

        void set_event_observer(tcp_client_observer_interface*) override;

    protected:
        //
        // -- tcp_client_observer_interface --
        //

        void on_close(bool transmission_error) override;
        void on_connect() override;
        void on_data(std::string& data) override;
        void on_end() override;
        void on_error(const std::string& message) override;
        void on_lookup(
            std::string& error,
            std::string& address,
            address_family family,
            std::string& host) override;
        void on_ready() override;
        void on_timeout() override;

    private:
        //
        // TODO: move variables to context pointer
        //

        tcp_client_t m_base;
        std::shared_ptr<gnutls_session_int> m_session;
        std::shared_ptr<gnutls_certificate_credentials_st> m_x509_credentials;
        tcp_client_observer_interface* m_fast_events;
        std::string m_server_name;
        std::string m_recv_buffer;
        std::mutex m_gnutls_mtx;
        bool m_cert_verify_enabled;
        bool m_update_handshake;

        void _M_init_gnutls_session();
        void _M_emit_error_code(std::string message, int code);
        void _M_fatal_error(int code);
        void _M_try_handshake();

        friend ssize_t gnutls_secure_client_send_callback(
            gnutls_transport_ptr_t, const void*, size_t);
        friend ssize_t gnutls_secure_client_recv_callback(
            gnutls_transport_ptr_t, void*, size_t);

        inline void _M_set_verify_cert();
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_SECURE_CLIENT_H */
