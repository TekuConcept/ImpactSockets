/**
 * Created by TekuConcept on March 7, 2021
 */

#ifndef IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H
#define IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H

#include <memory>
#include <string>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "interfaces/secure_x509_certificate_interface.h"
#include "interfaces/secure_datagram_interface.h"
#include "interfaces/event_loop_interface.h"
#include "sockets/gnutls_x509_certificate.h"

namespace impact {

    class gnutls_secure_datagram //:
        // public secure_datagram_interface,
        // protected udp_socket_observer_interface
    {
    public:
        // gnutls_secure_datagram(udp_socket_t base =
        //     default_event_loop()->create_udp_socket());
        ~gnutls_secure_datagram() = default;

        // prevent copying
        gnutls_secure_datagram(const gnutls_secure_datagram&) = delete;
        gnutls_secure_datagram& operator=(const gnutls_secure_datagram&) = delete;

        //
        // -- secure_datagram_interface --
        //

        // const std::string& server_name() const override;
        // void server_name(std::string host) override;
        // bool cert_verify_enabled() const override;
        // void cert_verify_enabled(bool enabled) override;

        //
        // -- secure_x509_certificate_interface --
        //

        // void set_x509_trust(
        //     std::string trust,
        //     secure_format_t format = secure_format_t::PEM) override;
        // void set_x509_cert_revoke_list(
        //     std::string crl,
        //     secure_format_t format = secure_format_t::PEM) override;
        // // ocsp: online certificate status protocol
        // void set_x509_ocsp_request_file(
        //     std::string ocsp_request_file,
        //     size_t index = 0) override;
        // void set_x509_credentials(
        //     std::string key,
        //     std::string certificate,
        //     secure_format_t format = secure_format_t::PEM) override;

        //
        // -- udp_socket_interface --
        //

        // int descriptor() const override;

        // udp_address_t address() const override;
        // udp_address_t remote_address() const override;
        // // size_t recv_buffer_size() const override;
        // // size_t send_buffer_size() const override;
        // // void recv_buffer_size(size_t size) override;
        // // void send_buffer_size(size_t size) override;

        // void add_membership(
        //     std::string address,
        //     std::string interface = std::string()) override;
        // void drop_membership(
        //     std::string address,
        //     std::string interface = std::string()) override;
        // // void add_source_specific_membership(
        // //     std::string source_address,
        // //     std::string group_address,
        // //     std::string interface = std::string()) override;
        // // void drop_source_specific_membership(
        // //     std::string source_address,
        // //     std::string group_address,
        // //     std::string interface = std::string()) override;

        // void bind(
        //     unsigned short port = 0,
        //     std::string address = "127.0.0.1",
        //     event_emitter::callback_t cb = nullptr) override;
        // void close(event_emitter::callback_t cb = nullptr) override;
        // // void connect(
        // //     unsigned short port = 0,
        // //     std::string address = "127.0.0.1",
        // //     event_emitter::callback_t cb = nullptr) override;
        // // void disconnect() override;

        // void send(
        //     std::string message,
        //     size_t offset,
        //     size_t length,
        //     unsigned short port = 0,
        //     std::string address = "127.0.0.1",
        //     event_emitter::callback_t cb = nullptr) override;

        // void pause() override;
        // void resume() override;

        // void set_broadcast(bool flag) override;
        // void set_multicast_interface(std::string interface) override;
        // void set_multicast_loopback(bool flag) override;
        // void set_multicast_ttl(unsigned char time_to_live) override;
        // void set_ttl(unsigned char time_to_live) override;

        // void set_event_observer(udp_socket_observer_interface*) override;

    protected:
        //
        // -- udp_socket_observer_interface --
        //

        // void on_close() override;
        // void on_connect() override;
        // void on_error(const std::string& message) override;
        // void on_listening() override;
        // void on_message(
        //     const std::string& data,
        //     const udp_address_t& address) override;

    private:
        udp_socket_t m_base;
        gnutls_x509_certificate m_certificate;
        udp_socket_observer_interface* m_fast_events;
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H */
