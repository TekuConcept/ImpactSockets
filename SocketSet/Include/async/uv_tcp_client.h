/**
 * Created by TekuConcept on February 11, 2021
 */

#ifndef IMPACT_UV_TCP_CLIENT_H
#define IMPACT_UV_TCP_CLIENT_H

#include <memory>
#include <atomic>
#include <future>
#include <vector>
#include "uv.h"
#include "interfaces/tcp_client_interface.h"
#include "interfaces/uv_node_interface.h"
#include "async/uv_event_loop.h"

namespace impact {

    class uv_tcp_server;
    class uv_tcp_client :
        public tcp_client_interface,
        public uv_child_interface,
        protected tcp_client_observer_interface
    {
    public:
        uv_tcp_client(uv_event_loop* event_loop);
        ~uv_tcp_client();

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
        tcp_client_interface* destroy(std::string error = std::string()) override;
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

        void send_signal(uv_node_signal_t op) override;

    protected:
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
        uv_event_loop*                 m_event_loop;
        tcp_address_t                  m_address;
        tcp_address_t                  m_local_address;
        tcp_address_t                  m_remote_address;
        struct addrinfo                m_hints;
        size_t                         m_bytes_read;
        size_t                         m_bytes_written;
        size_t                         m_timeout;
        bool                           m_has_timeout;
        std::string                    m_encoding;
        etimer_id_t                    m_timeout_handle;
        uv_tcp_server*                 m_server;
        uv_stream_t*                   m_stream;
        std::shared_ptr<uv_tcp_t>      m_handle;
        std::vector<char*>             m_malloc_buffers;
        tcp_client_observer_interface* m_fast_events;
        std::atomic<ready_state_t>     m_ready_state;

        struct write_context_t {
            uv_tcp_client* client;
            std::string data;
            event_emitter::callback_t cb;
        };

        uv_tcp_client(
            uv_event_loop*,
            uv_tcp_server*,
            std::shared_ptr<uv_tcp_t>);

        void _M_init(uv_event_loop* __event_loop);
        void _M_set_keepalive(bool, unsigned int);
        void _M_set_no_delay(bool);
        void _M_connect(std::string);
        void _M_connect(unsigned short, std::string);
        void _M_connect(const struct sockaddr*);
        void _M_pause();
        void _M_resume();
        bool _M_write(std::string, std::string, event_emitter::callback_t);
        void _M_set_timeout(unsigned int);
        void _M_end(std::string, std::string);
        void _M_destroy(std::string);
        void _M_emit_error_code(std::string, int) const;
        void _M_update_addresses();
        void _M_fill_address_info(const struct sockaddr*, tcp_address_t*);
        void _M_on_close();
        char* _M_alloc_buffer(size_t size);
        void _M_free_buffer(char* buffer);

        static void _S_on_path_resolved_callback(
            uv_getaddrinfo_t*, int, struct addrinfo*);
        static void _S_on_connect_callback(uv_connect_t*, int);
        static void _S_on_data_callback(uv_stream_t*, ssize_t, const uv_buf_t*);
        static void _S_on_write_callback(uv_write_t*, int);
        static void _S_on_shutdown_callback(uv_shutdown_t*, int);
        static void _S_on_close_callback(uv_handle_t* __handle);
        static void _S_alloc_buffer_callback(uv_handle_t*, size_t, uv_buf_t*);

        // enable constructing special-case uv_tcp_client from server
        // this is only meant to allow the server to call the private
        // uv_tcp_client constructor and nothing else
        friend class uv_tcp_server;
    };

} /* namespace impact */

#endif /* IMPACT_UV_TCP_CLIENT_H */
