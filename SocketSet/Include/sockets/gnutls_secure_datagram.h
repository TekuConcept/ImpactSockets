/**
 * Created by TekuConcept on March 16, 2021
 */

#ifndef IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H
#define IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H

#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <map>

#include <gnutls/gnutls.h>
#include <gnutls/dtls.h>

#include "interfaces/secure_datagram_interface.h"
#include "interfaces/secure_x509_certificate_interface.h"
#include "interfaces/event_loop_interface.h"
#include "sockets/gnutls_x509_certificate.h"

namespace impact {

    class gnutls_secure_datagram :
        public secure_datagram_interface,
        protected udp_socket_observer_interface
    {
    public:
        typedef std::shared_ptr<gnutls_session_int> session_t;
        typedef std::shared_ptr<gnutls_priority_st> priority_t;
        typedef std::shared_ptr<gnutls_dtls_prestate_st> prestate_t;
        typedef std::shared_ptr<gnutls_datum_t> cookie_t;

        gnutls_secure_datagram(udp_socket_t base =
            default_event_loop()->create_udp_socket());

        //
        // -- secure_datagram_interface --
        //

        size_t mtu() const override;
        void mtu(size_t value) override;
        void create(udp_address_t address) override;
        void begin(udp_address_t address) override;
        void end(udp_address_t address) override;
        const std::string& server_name(udp_address_t address) override;
        void server_name(udp_address_t address, std::string host) override;
        bool cert_verify_enabled(udp_address_t address) override;
        void cert_verify_enabled(udp_address_t address, bool enabled) override;

        void enable_server(bool enabled) override;

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
        // -- udp_socket_interface --
        //

        int descriptor() const override;

        udp_address_t address() const override;
        udp_address_t remote_address() const override;

        void add_membership(
            std::string address,
            std::string interface = std::string()) override;
        void drop_membership(
            std::string address,
            std::string interface = std::string()) override;
        // void add_source_specific_membership(
        //     std::string source_address,
        //     std::string group_address,
        //     std::string interface = std::string()) override;
        // void drop_source_specific_membership(
        //     std::string source_address,
        //     std::string group_address,
        //     std::string interface = std::string()) override;

        void bind(
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;
        void close(event_emitter::callback_t cb = nullptr) override;
        // void connect(
        //     unsigned short port = 0,
        //     std::string address = "127.0.0.1",
        //     event_emitter::callback_t cb = nullptr) override;
        // void disconnect() override;

        void send(
            std::string message,
            size_t offset,
            size_t length,
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;

        void pause() override;
        void resume() override;

        void set_broadcast(bool flag) override;
        void set_multicast_interface(std::string interface) override;
        void set_multicast_loopback(bool flag) override;
        void set_multicast_ttl(unsigned char time_to_live) override;
        void set_ttl(unsigned char time_to_live) override;

        void set_event_observer(udp_socket_observer_interface*) override;

    protected:
        //
        // -- udp_socket_observer_interface --
        //

        void on_close() override;
        void on_connect() override;
        void on_error(const std::string& message) override;
        void on_listening() override;
        void on_message(
            const std::string& data,
            const udp_address_t& address) override;

    private:
        udp_socket_t m_base;
        udp_socket_observer_interface* m_fast_events;
        gnutls_x509_certificate m_server_certificate;
        std::atomic<bool> m_server_enabled;
        priority_t m_priority_cache;
        cookie_t m_cookie_key;
        size_t m_mtu;
        secure_state_t m_state;

        struct route_context {
            session_t session;
            gnutls_x509_certificate certificate;
            prestate_t prestate;
            std::string server_name;
            std::string recv_buffer;
            bool cert_verify_enabled;
            secure_state_t state;
            udp_address_t address;
            udp_socket_t socket;
        };
        typedef std::shared_ptr<route_context> route_t;

        route_t m_loopback;
        std::map<udp_address_t, route_t> m_routes;
        std::mutex m_route_mtx;

        void _M_emit_error_code(std::string, int);
        inline void _M_emit_error_message(std::string);
        void _M_fatal_error(route_t, int);
        route_t _M_find_route(udp_address_t);
        void _M_init_server_info();
        void _M_init_loopback();
        void _M_init_gnutls_session(
            route_t, secure_connection_type_t,
            priority_t, prestate_t);
        void _M_close(route_t);
        inline void _M_set_verify_cert(route_t);
        void _M_try_handshake(route_t);
        inline void _M_remove_closed_routes();
        inline void _M_process_message(std::string, route_t);
        inline route_t _M_create_server_agent(std::string, udp_address_t);

        static ssize_t _S_on_send_callback(
            gnutls_transport_ptr_t, const void*, size_t);
        static ssize_t _S_on_recv_callback(
            gnutls_transport_ptr_t, void*, size_t);
    };

} /* namespace impact */

#endif /* IMPACT_SOCKETS_GNUTLS_SECURE_DATAGRAM_H */
