/**
 * Created by TekuConcept on March 4, 2021
 */

#ifndef IMPACT_UV_UDP_SOCKET_H
#define IMPACT_UV_UDP_SOCKET_H

#include <memory>
#include <vector>
#include <future>
#include <string>
#include "interfaces/udp_socket_interface.h"
#include "interfaces/uv_node_interface.h"
#include "async/uv_event_loop.h"

namespace impact {

    class uv_udp_socket :
        public udp_socket_interface,
        public uv_child_interface,
        protected udp_socket_observer_interface
    {
    public:
        uv_udp_socket(uv_event_loop* event_loop);
        ~uv_udp_socket();

        udp_address_t address() const override;
        udp_address_t remote_address() const override;
        // size_t recv_buffer_size() const override;
        // size_t send_buffer_size() const override;
        // void recv_buffer_size(size_t size) override;
        // void send_buffer_size(size_t size) override;

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
        // void connect(
        //     unsigned short port = 0,
        //     std::string address = "127.0.0.1",
        //     event_emitter::callback_t cb = nullptr) override;
        // void disconnect() override;
        void bind(
            unsigned short port = 0,
            std::string address = "127.0.0.1",
            event_emitter::callback_t cb = nullptr) override;
        void close(event_emitter::callback_t cb = nullptr) override;
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

        void send_signal(uv_node_signal_t op) override;

    protected:
        void on_close() override;
        void on_connect() override;
        void on_error(const std::string& message) override;
        void on_listening() override;
        void on_message(
            const std::string& data,
            const udp_address_t& address) override;

    private:
        uv_event_loop*                 m_event_loop;
        udp_address_t                  m_address;
        udp_address_t                  m_remote_address;
        struct addrinfo                m_hints;
        std::shared_ptr<uv_udp_t>      m_handle;
        std::vector<char*>             m_malloc_buffers;
        udp_socket_observer_interface* m_fast_events;
        // std::atomic<size_t> m_send_buffer_size;
        // std::atomic<size_t> m_recv_buffer_size;

        struct write_context_t {
            uv_udp_socket* socket;
            std::string data;
            event_emitter::callback_t cb;
        };

        void _M_add_membership(const std::string&, const std::string&);
        void _M_drop_membership(const std::string&, const std::string&);
        // void _M_add_source_specific_membership(
        //     const std::string&, const std::string&, const std::string&);
        // void _M_drop_source_specific_membership(
        //     const std::string&, const std::string&, const std::string&);
        // void _M_connect(unsigned short, const std::string&);
        // void _M_disconnect();
        void _M_bind(unsigned short, const std::string&,
            event_emitter::callback_t);
        void _M_send(const std::string&, size_t, size_t,
            unsigned short, const std::string&, event_emitter::callback_t);
        void _M_pause();
        void _M_resume();
        void _M_set_broadcast(bool);
        void _M_set_multicast_interface(const std::string&);
        void _M_set_multicast_loopback(bool);
        void _M_set_multicast_ttl(unsigned char);
        void _M_set_ttl(unsigned char);
        void _M_destroy();

        void _M_emit_error_code(std::string, int);
        void _M_fill_address_info(const struct sockaddr*, udp_address_t*);
        int _M_info_to_address(unsigned short, const std::string&,
            struct sockaddr*);
        char* _M_alloc_buffer(size_t size);
        void _M_free_buffer(char* buffer);

        static void _S_on_close_callback(uv_handle_t* __handle);
        static void _S_on_send_callback(uv_udp_send_t* request, int status);
        static void _S_on_recv_callback(uv_udp_t*, ssize_t,
            const uv_buf_t*, const struct sockaddr*, unsigned);
        static void _S_alloc_buffer_callback(uv_handle_t*, size_t, uv_buf_t*);
    };

} /* namespace impact */

#endif /* IMPACT_UV_UDP_SOCKET_H */
