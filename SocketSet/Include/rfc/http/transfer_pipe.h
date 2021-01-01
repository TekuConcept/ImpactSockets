/**
 * Created by TekuConcept on October 4, 2019
 */

#ifndef IMPACT_HTTP_TRANSFER_PIPE_H
#define IMPACT_HTTP_TRANSFER_PIPE_H

#include <vector>
#include <memory>
#include <functional>

#include "rfc/http/transfer_coding.h"

namespace impact {
namespace http {

    class transfer_pipe {
    public:
        static std::string EOP; // end-of-payload

        class sink {
        public:
            virtual ~sink() = default;
            virtual void on_chunk(const std::string& chunk) = 0;
            virtual void on_error(const std::string& error) = 0;
            virtual void on_eop() = 0;
        };

        transfer_pipe();
        virtual ~transfer_pipe() = default;

        // Note: if multiple chunked codings are provided,
        // the last one will be used, and the rest will be
        // discarded.
        //
        // This function takes ownership of all pointers
        // even if they are discarded.
        void set_codings(std::initializer_list<transfer_coding*> codings);
        void set_codings(std::vector<std::unique_ptr<transfer_coding>> codings);
        inline const std::vector<std::unique_ptr<transfer_coding>>& codings()
            const { return m_codings_; }

        size_t send(const std::string& data);

        // sink is used until the end-of-payload message is sent;
        // when end-of-payload is sent, the sink is released and
        // another sink can be set thereafter; if a sink is already
        // set but a new one is set in its place, the previous sink's
        // on_error() callback is invoked
        void set_sink(sink* sink);
        inline bool has_sink() const { return m_sink_ != nullptr; }

        inline const std::string& eop() const
        { return transfer_pipe::EOP; }

    private:
        std::vector<std::unique_ptr<transfer_coding>> m_codings_;
        sink* m_sink_;

        void _M_push(transfer_coding*);
    };

}}

#endif
