/**
 * Created by TekuConcept on January 19, 2021
 */

#ifndef IMPACT_UV_TCP_SERVER_H
#define IMPACT_UV_TCP_SERVER_H

#include <memory>
#include <atomic>
#include <vector>
#include <future>
#include <mutex>
#include "uv.h"
#include "interfaces/tcp_server_interface.h"
#include "interfaces/uv_node_interface.h"
#include "async/uv_event_loop.h"

namespace impact {

    class uv_tcp_client;
    class uv_tcp_server :
        public tcp_server_interface,
        public uv_child_interface,
        protected tcp_server_observer_interface
    {
    public:
        uv_tcp_server(uv_event_loop* event_loop);
        ~uv_tcp_server();

        int descriptor() const override;

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

        void send_signal(uv_node_signal_t op) override;

    protected:
        void on_close() override;
        void on_connection(const tcp_client_t& connection) override;
        void on_error(const std::string& message) override;
        void on_listening() override;

    private:
        uv_event_loop*                 m_event_loop;
        tcp_server_observer_interface* m_fast_events;
        std::atomic<bool>              m_is_listening;
        size_t                         m_max_connections;
        tcp_address_t                  m_address;
        std::shared_ptr<uv_tcp_t>      m_handle;
        struct addrinfo                m_hints;
        std::vector<uv_tcp_client*>    m_connection_list;
        std::mutex                     m_list_mtx;

        void _M_close();
        void _M_listen(unsigned short port, std::string host);
        void _M_listen(std::string path);
        void _M_listen(const struct sockaddr*);
        void _M_emit_error_code(std::string, int) const;
        void _M_add_client_reference(uv_tcp_client*);
        void _M_remove_client_reference(uv_tcp_client*);
        size_t _M_client_reference_count();

        static void _S_on_close_callback(uv_handle_t*);
        static void _S_on_connection_callback(uv_stream_t*, int);
        static void _S_on_path_resolved_callback(
            uv_getaddrinfo_t*, int, struct addrinfo*);

        // only meant to allow clients created by the server to
        // remove themselves from the server's connection count
        // list and nothing else
        friend class uv_tcp_client;
    };

} /* namespace impact */

#endif /* IMPACT_UV_TCP_SERVER_H */
