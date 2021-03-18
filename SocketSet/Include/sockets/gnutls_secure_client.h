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

#include "sockets/gnutls_x509_certificate.h"
#include "interfaces/secure_client_interface.h"
#include "interfaces/tcp_client_interface.h"
#include "interfaces/event_loop_interface.h"

namespace impact {

    class gnutls_secure_client :
        public secure_client_interface,
        protected tcp_client_observer_interface
    {
    public:
        typedef std::shared_ptr<gnutls_session_int> session_t;
        typedef std::shared_ptr<gnutls_priority_st> priority_t;

        gnutls_secure_client(
            tcp_client_t base =
                default_event_loop()->create_tcp_client(),
            secure_connection_type_t connection_type =
                secure_connection_type_t::CLIENT,
            gnutls_x509_certificate certificate =
                gnutls_x509_certificate(),
            priority_t priority = nullptr);
        ~gnutls_secure_client() = default;

        // prevent copying
        gnutls_secure_client(const gnutls_secure_client&) = delete;
        gnutls_secure_client& operator=(const gnutls_secure_client&) = delete;

        //
        // -- secure_client_interface --
        //

        const std::string& server_name() const override;
        void server_name(std::string host) override;
        bool cert_verify_enabled() const override;
        void cert_verify_enabled(bool enabled) override;

        //
        // -- secure_x509_certificate_interface --
        //

        void set_x509_trust(
            std::string trust,
            secure_format_t format = secure_format_t::PEM) override;
        void set_x509_cert_revoke_list(
            std::string crl,
            secure_format_t format = secure_format_t::PEM) override;
        // ocsp: online certificate status protocol
        void set_x509_ocsp_request_file(
            std::string ocsp_request_file,
            size_t index = 0) override;
        void set_x509_credentials(
            std::string key,
            std::string certificate,
            secure_format_t format = secure_format_t::PEM) override;

        //
        // -- tcp_client_interface --
        //

        int descriptor() const override;

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
        tcp_client_t m_base;
        session_t m_session;
        gnutls_x509_certificate m_certificate;
        tcp_client_observer_interface* m_fast_events;
        std::string m_server_name;
        std::string m_recv_buffer;
        std::mutex m_gnutls_mtx;
        bool m_cert_verify_enabled;
        secure_state_t m_state;

        void _M_init_gnutls_session(priority_t, secure_connection_type_t);
        void _M_emit_error_code(std::string message, int code);
        inline void _M_emit_error_message(std::string message);
        void _M_fatal_error(int code);
        void _M_try_handshake();
        void _M_destroy();
        void _M_end();
        inline void _M_set_verify_cert();

        static ssize_t _S_on_send_callback(
            gnutls_transport_ptr_t, const void*, size_t);
        static ssize_t _S_on_recv_callback(
            gnutls_transport_ptr_t, void*, size_t);
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_SECURE_CLIENT_H */
