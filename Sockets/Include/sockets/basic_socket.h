/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_BASIC_SOCKET_H_
#define _IMPACT_BASIC_SOCKET_H_


#include <string>
#include <future>

#include "sockets/types.h"

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
		void close() /* throw(io_error), throw(runtime_error) */;

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

		// communication / delivery
		void bind(unsigned short port) /* throw(io_error), throw(runtime_error) */;
		void bind(const std::string& address, unsigned short port = 0)
			/* throw(io_error), throw(runtime_error) */;
		void connect(unsigned short port, const std::string& address = "localhost")
			/* throw(io_error), throw(runtime_error) */;
		void listen(int backlog = 5)
			/* throw(io_error), throw(runtime_error) */;
		basic_socket accept()
			/* throw(io_error), throw(runtime_error) */;
		void shutdown(socket_channel channel = socket_channel::BOTH)
			/* throw(io_error), throw(runtime_error) */;
		void group(std::string multicast_name, group_application method)
			/* throw(io_error), throw(runtime_error) */;
		void keepalive(struct keep_alive_options options)
			/* throw(io_error), throw(runtime_error) */;
		int send(const void* buffer, int length,
			message_flags flags = message_flags::NONE)
			/* throw(io_error), throw(runtime_error) */;
		int sendto(const void* buffer, int length, unsigned short port,
			const std::string& address,
			message_flags flags = message_flags::NONE)
			/* throw(io_error), throw(runtime_error) */;
		int recv(void* buffer, int length,
			message_flags flags = message_flags::NONE)
			/* throw(io_error), throw(runtime_error) */;
		int recvfrom(void* buffer, int length, unsigned short* port,
			std::string* address, message_flags flags = message_flags::NONE)
			/* throw(io_error), throw(runtime_error) */;

		// async communication
		// std::future& basic_socket.accept_async();
		// std::future<buffer_data>& send_async(const void* buffer, int length,
		// 	message_flags flags = message_flags::NONE);
		// std::future<async_buffer_data>& sendto_async(const void* buffer, int length,
		// 	unsigned short port, const std::string& address,
		// 	message_flags flags = message_flags::NONE);
		// std::future& basic_socket.recv_async();
		// std::future& basic_socket.recvfrom_async();

		// miscillaneous
		std::string local_address() /* throw(io_error), throw(runtime_error) */;
		unsigned short local_port() /* throw(io_error), throw(runtime_error) */;
		std::string peer_address() /* throw(io_error), throw(runtime_error) */;
		unsigned short peer_port() /* throw(io_error), throw(runtime_error) */;
		void broadcast(bool enabled) /* throw(io_error), throw(runtime_error) */;
		void multicast_ttl(unsigned char time_to_live = 1)
			/* throw(io_error), throw(runtime_error) */;
		void reuse_address(bool enabled)
			/* throw(io_error), throw(runtime_error) */;

		friend basic_socket make_socket(socket_domain, socket_type, socket_protocol);
		friend basic_socket make_tcp_socket();
		friend basic_socket make_udp_socket();

	private:
		struct basic_socket_info {
			long            ref_count;
			bool            wsa;
			int             descriptor;
			socket_domain   domain;
			socket_type     type;
			socket_protocol protocol;
		};

		struct basic_socket_info* m_info_;

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
