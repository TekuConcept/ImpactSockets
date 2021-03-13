/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_SOCKETS_GNUTLS_SECURE_SERVER_H
#define IMPACT_SOCKETS_GNUTLS_SECURE_SERVER_H

#include <memory>
#include <string>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "sockets/gnutls_x509_certificate.h"
#include "interfaces/secure_server_interface.h"
#include "interfaces/tcp_server_interface.h"
#include "interfaces/event_loop_interface.h"

namespace impact {

    class gnutls_secure_server :
        public secure_server_interface,
        protected tcp_server_observer_interface
    {
    public:
        gnutls_secure_server(
            tcp_server_t base =
                default_event_loop()->create_tcp_server(),
            gnutls_x509_certificate certificate =
                gnutls_x509_certificate());
        ~gnutls_secure_server() = default;

        // prevent copying
        gnutls_secure_server(const gnutls_secure_server&) = delete;
        gnutls_secure_server& operator=(const gnutls_secure_server&) = delete;

        //
        // -- secure_server_interface --
        //

        //
        // -- secure_x509_certificate_interface --
        //

        void set_x509_trust(
            std::string trust,
            secure_format_t format = secure_format_t::PEM) override;
        void set_x509_cert_revoke_list(
            std::string crl,
            secure_format_t format = secure_format_t::PEM) override;
        void set_x509_ocsp_request_file(
            std::string ocsp_request_file,
            size_t index = 0) override;
        void set_x509_credentials(
            std::string key,
            std::string certificate,
            secure_format_t format = secure_format_t::PEM) override;

        //
        // -- tcp_server_interface --
        //

        tcp_address_t address() const override;
        bool listening() const override;
        size_t max_connections() const override;
        void max_connections(size_t value) override;

        void close(event_emitter::callback_t cb = nullptr) override;
        void listen(
            std::string path,
            event_emitter::callback_t cb = nullptr) override;
        void listen(
            unsigned short port = 0,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;

        void set_event_observer(tcp_server_observer_interface*) override;

    protected:
        //
        // -- tcp_server_observer_interface --
        //

        void on_close() override;
        void on_connection(const tcp_client_t& connection) override;
        void on_error(const std::string& message) override;
        void on_listening() override;

    private:
        tcp_server_t m_base;
        gnutls_x509_certificate m_certificate;
        std::shared_ptr<gnutls_priority_st> m_priority_cache;
        tcp_server_observer_interface* m_fast_events;

        void _M_init_gnutls_session();
        void _M_emit_error_code(std::string, int);
        inline void _M_emit_error_message(std::string message);
        gnutls_x509_crt_fmt_t _M_get_gnutls_format(secure_format_t);
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_SECURE_SERVER_H */
