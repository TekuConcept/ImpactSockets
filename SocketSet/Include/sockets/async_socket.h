/**
 * Created by TekuConcept on January 8, 2021
 */

#ifndef IMPACT_ASYNC_SOCKET_H
#define IMPACT_ASYNC_SOCKET_H

namespace impact {

    class async_socket {
    public:
        /*
        async_socket();
        async_socket(const async_socket&) = default;
        async_socket(async_socket&&) = default;
        async_socket(
            address_family domain,
            socket_type type,
            internet_protocol proto,
            loop = default);
        ~async_socket();

        async_socket& operator=(const async_socket&) = default;
        async_socket& operator=(async_socket&&) = default;

        async_socket create_tcp(loop = default);
        async_socket create_udp(loop = default);

        inline async_socket_observer* observer() const;
        inline observer(async_socket_observer* __observer)
        { m_observer = __observer; }

        -----------------------

        class async_socket_observer {
        public:
            virtual ~async_socket_observer() = default;

            on_close()
            on_connection()
            on_error()
            on_listening()
            on_connect()
            on_data()
            on_drain()
            on_end()
            on_lookup()
            on_ready()
            on_timeout()

            on_read()
            on_write()
        };

        -----------------------
        async_socket asoc = loop->add_socket((basic_socket)(bsoc))
        loop->remove_socket(asoc)

        async_socket asoc = ((basic_socket)(bsoc)).make_async(loop = default)
        */

    private:
    };

} /* impact */

#endif /* IMPACT_ASYNC_SOCKET_H */
