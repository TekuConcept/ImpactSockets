/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_BASIC_SOCKET_H_
#define _IMPACT_BASIC_SOCKET_H_

#include "sockets/types.h"

#include <string>

namespace impact {
  class basic_socket {
  public:
    enum {
      invalid = -1
    };

    // constructors
    basic_socket();
    basic_socket(const basic_socket& r) /* throw(std::runtime_error) */;
    basic_socket(basic_socket&& r) /* throw(std::runtime_error) */;

    // destructors
    virtual ~basic_socket();

    // file operators
    void close();

    // assignment
    basic_socket& operator=(const basic_socket& r) /* throw(io_error) */;
    basic_socket& operator=(basic_socket&& r) /* throw(io_error) */;

    // observers
    long use_count() const noexcept;
    int get() const noexcept;
    socket_domain domain() const noexcept;
    socket_type type() const noexcept;
    socket_protocol protocol() const noexcept;
    explicit operator bool() const noexcept;

    // communication
    void connect(unsigned short port, const std::string& address = "localhost")
      /* throw(io_error) */;
    void listen(int backlog = 5)
      /* throw(io_error) */;
    basic_socket accept()
      /* throw(io_error) */;
    void shutdown(socket_channel channel = socket_channel::BOTH)
      /* throw(io_error) */;
    void group(std::string multicast_name, group_application method)
      /* throw(io_error) */;
    void keepalive(struct keep_alive_options options)
      /* throw(io_error) */;
    void send(const void* buffer, int length,
      message_flags flags = message_flags::NONE)
      /* throw(io_error) */;
    int sendto(const void* buffer, int length, unsigned short port,
      const std::string& address,
      message_flags flags = message_flags::NONE)
      /* throw(io_error) */;
    int recv(void* buffer, int length,
      message_flags flags = message_flags::NONE)
      /* throw(io_error) */;
    int recvfrom(void* buffer, int length, unsigned short& port,
      std::string& address, message_flags flags = message_flags::NONE)
      /* throw(io_error) */;

    // miscillaneous
    void local_port(unsigned short port) /* throw(io_error) */;
    void local_address_port(const std::string& address, unsigned short port = 0)
      /* throw(io_error) */;
    std::string local_address() /* throw(io_error) */;
    unsigned short local_port() /* throw(io_error) */;
    std::string peer_address() /* throw(io_error) */;
    unsigned short peer_port() /* throw(io_error) */;
    void broadcast(bool enabled) /* throw(io_error) */;
    void multicast_ttl(unsigned char time_to_live = 1) /* throw(io_error) */;

    friend basic_socket make_socket(socket_domain, socket_type, socket_protocol);
    friend basic_socket make_tcp_socket();
    friend basic_socket make_udp_socket();

  private:
      long* m_ref_;
      bool* m_wsa_;
      int*  m_descriptor_;

      socket_domain   m_domain_;
      socket_type     m_type_;
      socket_protocol m_protocol_;

      unsigned short _M_resolve_service(const std::string& __service,
        const std::string& __protocol = "tcp");

      void _M_copy(const basic_socket& __rhs);
      void _M_move(basic_socket&& __rhs);
      void _M_dtor();
  };

  basic_socket make_socket(socket_domain domain, socket_type type, socket_protocol proto) /* throw(io_error) */;
  basic_socket make_tcp_socket() /* throw(io_error) */;
  basic_socket make_udp_socket() /* throw(io_error) */;
}

#endif
