/**
 * Created by TekuConcept on January 19, 2021
 */

#ifndef IMPACT_UV_TCP_SERVER_H
#define IMPACT_UV_TCP_SERVER_H

#include <memory>
#include <atomic>
#include <vector>
#include <future>
#include "uv.h"
#include "sockets/tcp_server_interface.h"
#include "async/uv_event_loop.h"

namespace impact {

    class uv_tcp_server : public tcp_server_interface {
    public:
        uv_tcp_server(
            std::shared_ptr<struct uv_event_loop::context_t> event_loop);
        ~uv_tcp_server();

        tcp_address_t address() const override;
        bool listening() const override;
        size_t max_connections() const override;
        void max_connections(size_t value) override;

        void close(event_emitter::callback_t cb = nullptr) override;
        void listen(
            std::string path,
            event_emitter::callback_t cb = nullptr) override;
        void listen(
            unsigned short port,
            event_emitter::callback_t cb) override;
        void listen(
            unsigned short port = 0,
            std::string host = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;

    private:
        std::shared_ptr<struct uv_event_loop::context_t> m_event_loop;
        std::atomic<bool>   m_is_listening;
        size_t              m_max_connections;
        tcp_address_t       m_address;
        uv_tcp_t            m_handle;
        struct addrinfo     m_hints;
        std::atomic<size_t> m_client_count;

        enum class request_type { CLOSE, LISTEN1, LISTEN2 };
        struct async_request_t {
            request_type type;
            std::string host;
            unsigned short port;
            std::promise<void>* promise;

            async_request_t();
        };
        uv_rwlock_t                         m_lock;
        uv_async_t                          m_async_handle;
        std::vector<struct async_request_t> m_requests;

        void _M_close();
        void _M_close_async(bool blocking = false);
        void _M_listen(unsigned short port, std::string host);
        void _M_listen(std::string path);
        void _M_listen(const struct sockaddr*);
        void _M_listen_async(unsigned short port, std::string host);
        void _M_listen_async(std::string path);
        void _M_emit_listen_error(int status);

        friend void uv_tcp_server_on_close(uv_handle_t*);
        friend void uv_tcp_server_on_connection(uv_stream_t*, int);
        friend void uv_tcp_server_on_path_resolved(
            uv_getaddrinfo_t*, int, struct addrinfo*);
        friend void uv_tcp_server_async_callback(uv_async_t*);
    };

} /* namespace impact */

#endif /* IMPACT_UV_TCP_SERVER_H */