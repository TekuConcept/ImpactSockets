/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "SocketTypes.h"
#include <string>

namespace Impact {
  class basic_socket {
    long* _ref_;
    int _descriptor_;
    SocketDomain _domain_;
    SocketType _type_;
    SocketProtocol _protocol_;

    unsigned short resolve_service(const std::string& service,
      const std::string& protocol = "tcp");

    void copy(const basic_socket&);
    void move(basic_socket&&);
    void dtor();

  public:
    // constructors
    basic_socket() /* throw(io_error) */;
    basic_socket(const basic_socket& r)
      /* throw(std::runtime_error) */;
    basic_socket(basic_socket&& r)
      /* throw(std::runtime_error) */;

    // destructors
    ~basic_socket();

    // file operators
    void close();

    // assignment
    basic_socket& operator=(const basic_socket& r) /* throw(io_error) */;
    basic_socket& operator=(basic_socket&& r) /* throw(io_error) */; // close socket if last to own and open

    // observers
    long use_count() const noexcept;
    int get() const noexcept;
    SocketDomain domain() const noexcept;
    SocketType type() const noexcept;
    SocketProtocol protocol() const noexcept;
    explicit operator bool() const noexcept; // get() != INVALID_SOCKET

    // communication
    void connect(unsigned short port, const std::string& address)
      /* throw(io_error) */;
    void listen(int backlog = 5)
      /* throw(io_error) */;
    basic_socket accept()
      /* throw(io_error) */;
    void shutdown(SocketChannel channel = SocketChannel::BOTH)
      /* throw(io_error) */;
    void group(std::string multicastName, GroupApplication method)
      /* throw(io_error) */;
    void keepalive(KeepAliveOptions options)
      /* throw(io_error) */;
    void send(const void* buffer, int length,
      MessageFlags flags = MessageFlags::NONE)
      /* throw(io_error) */;
    int sendto(const void* buffer, int length, unsigned short port,
      const std::string& address, MessageFlags flags = MessageFlags::NONE);
      /* throw(io_error) */;
    int recv(void* buffer, int length, MessageFlags flags = MessageFlags::NONE)
      /* throw(io_error) */;
    int recvfrom(void* buffer, int length, unsigned short& port,
      std::string& address, MessageFlags flags = MessageFlags::NONE)
      /* throw(io_error) */;

    // miscillaneous
	void local_port(unsigned short port) /* throw(io_error) */;
	void local_address_port(const std::string& address,
		unsigned short port = 0) /* throw(io_error) */;
    std::string local_address() /* throw(io_error) */;
    unsigned short local_port() /* throw(io_error) */;
    std::string peer_address() /* throw(io_error) */;
    unsigned short peer_port() /* throw(io_error) */;
    void broadcast(bool enabled) /* throw(io_error) */;
    void multicast_ttl(unsigned char timeToLive = 1)
      /* throw(io_error) */;

    friend basic_socket make_socket(SocketDomain, SocketType, SocketProtocol);
  };

  basic_socket make_socket(SocketDomain domain, SocketType type,
    SocketProtocol proto) /* throw(io_error) */;
}

#endif
