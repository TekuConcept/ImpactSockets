/*
*   C++ sockets on Unix and Windows
*   Copyright (C) 2002
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Modified by TekuConcept
*/

#ifndef __PRACTICALSOCKET_INCLUDED__
#define __PRACTICALSOCKET_INCLUDED__

#include <iostream>
#include <string>            // For string
#include <cstring>           // For strerror, atoi, and memset
#include <exception>         // For exception class
#include <vector>

#if defined(_MSC_VER)
#include <winsock2.h>
#define SOC_EXCEPTION ...
#else
#include <sys/poll.h>    // For struct pollfd, poll()
#define SOC_EXCEPTION SocketException
#endif

#define string std::string
#define exception std::exception

namespace Impact {
	/**
	*   Holds onto the socket descriptor and allows for
	*	class-restricted socket manipulation.
	*/
	class SocketHandle {
	protected:
		int descriptor;
		friend class Socket;
		friend class SocketPollToken;
		friend class CommunicatingSocket;
		friend class TCPServerSocket;
		friend class UDPSocket;
	};

	/**
	*   SocketPollToken replaces the pollfd struct by using
	*   SocketHandles instead of raw socket descriptors.
	*/
	class SocketPollToken {
	protected:
		friend class Socket;
		std::vector<struct pollfd> _handles_;
	public:
		SocketPollToken();
		~SocketPollToken();
		/**
		* Adds a handle to the queue with the given events to listen for.
		*/
		void add(SocketHandle& handle, int events);
		/**
		* Removes the handle at the specified index by swapping it with
		* the last handle added. The index of the last handle then assumes
		* the index of the removed handle and the token's size decreases by 1.
		*/
		void remove(int idx);
		/**
		* Returns the number of handles in this token.
		*/
		unsigned int size() const;
		/**
		* Resets all the return events to 0.
		*/
		void reset();
		/**
		* Removes all handles from the token.
		*/
		void clear();
		/**
		* Access the return event for the handle.
		*/
		short int& operator[] (int);
	};

	/**
	*   KeepAlive Options
	*/
	typedef struct KeepAliveOptions {
		int idle;     /* Number of idle seconds before sending a KA probe.   */
		int interval; /* How often in seconds to resend an unacked KA probe. */
		int count;    /* How many times to resend a KA probe if previous
		                 probe was unacked.                                  */
	} KeepAliveOptions;

	/**
	*   Signals a problem with the execution of a socket call.
	*/
	class SocketException : public exception {
	public:
		/**
		*   Construct a SocketException with a explanatory message.
		*   @param message explanatory message
		*   @param incSysMsg true if system message (from strerror(errno))
		*   should be postfixed to the user provided message
		*/
		SocketException(const string &message, bool inclSysMsg = false) throw();

		/**
		*   Provided just to guarantee that no exceptions are thrown.
		*/
		~SocketException() throw();

		/**
		*   Get the exception message
		*   @return exception message
		*/
		const char *what() const throw();

	private:
		string userMessage;  // Exception message
	};

	/**
	*   Base class representing basic communication endpoint
	*/
	class Socket {
	public:
		/**
		*   Close and deallocate this socket
		*/
		~Socket();

		/**
		*   Get the handle of this socket
		*/
		SocketHandle& getHandle();

		/**
		*   Get the local address
		*   @return local address of socket
		*   @exception SocketException thrown if fetch fails
		*/
		string getLocalAddress() throw(SOC_EXCEPTION);

		/**
		*   Get the local port
		*   @return local port of socket
		*   @exception SocketException thrown if fetch fails
		*/
		unsigned short getLocalPort() throw(SOC_EXCEPTION);

		/**
		*   Set the local port to the specified port and the local address
		*   to any interface
		*   @param localPort local port
		*   @exception SocketException thrown if setting local port fails
		*/
		void setLocalPort(unsigned short localPort) throw(SOC_EXCEPTION);

		/**
		*   Set the local port to the specified port and the local address
		*   to the specified address.  If you omit the port, a random port
		*   will be selected.
		*   @param localAddress local address
		*   @param localPort local port
		*   @exception SocketException thrown if setting local port or address fails
		*/
		void setLocalAddressAndPort(const string &localAddress,
			unsigned short localPort = 0) throw(SOC_EXCEPTION);

		/**
		*   Resolve the specified service for the specified protocol to the
		*   corresponding port number in host byte order
		*   @param service service to resolve (e.g., "http")
		*   @param protocol protocol of service to resolve.  Default is "tcp".
		*/
		static unsigned short resolveService(const string &service,
			const string &protocol = "tcp");
		
		/**
		*   Configure TCP KEEPALIVE flag.
		*   @param handle Handle to the socket which to configure.
		*   @param enable True enables keep-alive.
		*/
		static void keepalive(SocketHandle& handle, bool enable=true) throw(SOC_EXCEPTION);
		static void keepalive(SocketHandle& handle, KeepAliveOptions options,
			bool enable=true) throw(SOC_EXCEPTION);

		/**
		*   Allows a program to monitor 'readability' multiple sockets.
		*   WARNING: Only tested on Linux
		*   @param handles SocketHandles to collection of sockets
		*   @param timeout Time in seconds before timing out
		*   @return 1 for success, 0 for timeout
		*/
		static int select(SocketHandle** handles, int length, unsigned int timeout);
		static int select(SocketHandle** handles, int length, struct timeval* timeout = NULL);

		/*
		*   Polls sockets for events.
		*   @param token Contains a list of SocketHandles and event information.
		*   @param timeout Time in milliseconds to poll. -1: indefinite, 0:
		*   return immediately
		*   @return 1 for success, 0 for timeout, -1 if an error occurred.
		*   @exception SocketException thrown if poll failed
		*/
		static int poll(SocketPollToken& token, int timeout = -1);

	private:
		// Prevent the user from trying to use value semantics on this object
		Socket(const Socket &sock);
		void operator=(const Socket &sock) = delete;

	protected:
		SocketHandle handle;
		Socket(int type, int protocol) throw(SOC_EXCEPTION);
		Socket(int sockDesc);
	};

	/**
	*   Socket which is able to connect, send, and receive
	*/
	class CommunicatingSocket : public Socket {
	public:
		/**
		*   Establish a socket connection with the given foreign
		*   address and port
		*   @param foreignAddress foreign address (IP address or name)
		*   @param foreignPort foreign port
		*   @exception SocketException thrown if unable to establish connection
		*/
		void connect(const string &foreignAddress, unsigned short foreignPort)
			throw(SOC_EXCEPTION);

		void disconnect() throw(SOC_EXCEPTION);

		/**
		*   Write the given buffer to this socket.  Call connect() before
		*   calling send()
		*   @param buffer buffer to be written
		*   @param bufferLen number of bytes from buffer to be written
		*   @exception SocketException thrown if unable to send data
		*/
		void send(const void *buffer, int bufferLen) throw(SOC_EXCEPTION);
		void send(const void *buffer, int bufferLen, int flags) throw(SOC_EXCEPTION);

		/**
		*   Read into the given buffer up to bufferLen bytes data from this
		*   socket.  Call connect() before calling recv()
		*   @param buffer buffer to receive the data
		*   @param bufferLen maximum number of bytes to read into buffer
		*   @return number of bytes read, 0 for EOF, and -1 for error
		*   @exception SocketException thrown if unable to receive data
		*/
		int recv(void *buffer, int bufferLen) throw(SOC_EXCEPTION);
		int recv(void *buffer, int bufferLen, int flags) throw(SOC_EXCEPTION);

		/**
		*   Get the foreign address.  Call connect() before calling recv()
		*   @return foreign address
		*   @exception SocketException thrown if unable to fetch foreign address
		*/
		string getForeignAddress() throw(SOC_EXCEPTION);

		/**
		*   Get the foreign port.  Call connect() before calling recv()
		*   @return foreign port
		*   @exception SocketException thrown if unable to fetch foreign port
		*/
		unsigned short getForeignPort() throw(SOC_EXCEPTION);

	protected:
		CommunicatingSocket(int type, int protocol) throw(SOC_EXCEPTION);
		CommunicatingSocket(int newConnSD);
	};

	/**
	*   TCP socket for communication with other TCP sockets
	*/
	class TCPSocket : public CommunicatingSocket {
	public:
		/**
		*   Construct a TCP socket with no connection
		*   @exception SocketException thrown if unable to create TCP socket
		*/
		TCPSocket() throw(SOC_EXCEPTION);

		/**
		*   Construct a TCP socket with a connection to the given foreign address
		*   and port
		*   @param foreignAddress foreign address (IP address or name)
		*   @param foreignPort foreign port
		*   @exception SocketException thrown if unable to create TCP socket
		*/
		TCPSocket(const string &foreignAddress, unsigned short foreignPort)
			throw(SOC_EXCEPTION);

	private:
		// Access for TCPServerSocket::accept() connection creation
		friend class TCPServerSocket;
		TCPSocket(int newConnSD);
	};

	/**
	*   TCP socket class for servers
	*/
	class TCPServerSocket : public Socket {
	public:
		/**
		*   Construct a TCP socket for use with a server, accepting connections
		*   on the specified port on any interface
		*   @param localPort local port of server socket, a value of zero will
		*                   give a system-assigned unused port
		*   @param queueLen maximum queue length for outstanding
		*                   connection requests (default 5)
		*   @exception SocketException thrown if unable to create TCP server socket
		*/
		TCPServerSocket(unsigned short localPort, int queueLen = 5)
			throw(SOC_EXCEPTION);

		/**
		*   Construct a TCP socket for use with a server, accepting connections
		*   on the specified port on the interface specified by the given address
		*   @param localAddress local interface (address) of server socket
		*   @param localPort local port of server socket
		*   @param queueLen maximum queue length for outstanding
		*                   connection requests (default 5)
		*   @exception SocketException thrown if unable to create TCP server socket
		*/
		TCPServerSocket(const string &localAddress, unsigned short localPort,
			int queueLen = 5) throw(SOC_EXCEPTION);

		/**
		*   Blocks until a new connection is established on this socket or error
		*   @return new connection socket
		*   @exception SocketException thrown if attempt to accept a new connection fails
		*/
		TCPSocket *accept() throw(SOC_EXCEPTION);

	private:
		void setListen(int queueLen) throw(SOC_EXCEPTION);
	};

	/**
	*   UDP socket class
	*/
	class UDPSocket : public CommunicatingSocket {
	public:
		/**
		*   Construct a UDP socket
		*   @exception SocketException thrown if unable to create UDP socket
		*/
		UDPSocket() throw(SOC_EXCEPTION);

		/**
		*   Construct a UDP socket with the given local port
		*   @param localPort local port
		*   @exception SocketException thrown if unable to create UDP socket
		*/
		UDPSocket(unsigned short localPort) throw(SOC_EXCEPTION);

		/**
		*   Construct a UDP socket with the given local port and address
		*   @param localAddress local address
		*   @param localPort local port
		*   @exception SocketException thrown if unable to create UDP socket
		*/
		UDPSocket(const string &localAddress, unsigned short localPort)
			throw(SOC_EXCEPTION);

		/**
		*   Unset foreign address and port
		*   @return true if disassociation is successful
		*   @exception SocketException thrown if unable to disconnect UDP socket
		*/
		void disconnect() throw(SOC_EXCEPTION);

		/**
		*   Send the given buffer as a UDP datagram to the
		*   specified address/port
		*   @param buffer buffer to be written
		*   @param bufferLen number of bytes to write
		*   @param foreignAddress address (IP address or name) to send to
		*   @param foreignPort port number to send to
		*   @return true if send is successful
		*   @exception SocketException thrown if unable to send datagram
		*/
		void sendTo(const void *buffer, int bufferLen, const string &foreignAddress,
			unsigned short foreignPort) throw(SOC_EXCEPTION);

		/**
		*   Read read up to bufferLen bytes data from this socket.  The given buffer
		*   is where the data will be placed
		*   @param buffer buffer to receive data
		*   @param bufferLen maximum number of bytes to receive
		*   @param sourceAddress address of datagram source
		*   @param sourcePort port of data source
		*   @return number of bytes received and -1 for error
		*   @exception SocketException thrown if unable to receive datagram
		*/
		int recvFrom(void *buffer, int bufferLen, string &sourceAddress,
			unsigned short &sourcePort) throw(SOC_EXCEPTION);

		/**
		*   Set the multicast TTL
		*   @param multicastTTL multicast TTL
		*   @exception SocketException thrown if unable to set TTL
		*/
		void setMulticastTTL(unsigned char multicastTTL) throw(SOC_EXCEPTION);

		/**
		*   Join the specified multicast group
		*   @param multicastGroup multicast group address to join
		*   @exception SocketException thrown if unable to join group
		*/
		void joinGroup(const string &multicastGroup) throw(SOC_EXCEPTION);

		/**
		*   Leave the specified multicast group
		*   @param multicastGroup multicast group address to leave
		*   @exception SocketException thrown if unable to leave group
		*/
		void leaveGroup(const string &multicastGroup) throw(SOC_EXCEPTION);

	private:
		void setBroadcast();
	};
}

#undef exception
#undef string
#endif
