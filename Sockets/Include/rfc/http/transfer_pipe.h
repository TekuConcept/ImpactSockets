/**
 * Created by TekuConcept on October 4, 2019
 */

#ifndef _IMPACT_HTTP_TRANSFER_PIPE_H_
#define _IMPACT_HTTP_TRANSFER_PIPE_H_

#include <vector>
#include <memory>
#include <functional>

#include "rfc/http/transfer_coding.h"

namespace impact {
namespace http {

    class transfer_pipe {
    public:
        static std::string EOP; // end-of-payload

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

        // sink will be used until the end-of-payload signal is sent;
        // the sink needs to be set again to resend the payload or to
        // send a new payload
        inline void set_sink(std::function<void(const std::string&)> sink)
        { m_sink_ = sink; }

        inline const std::string& eop() const
        { return transfer_pipe::EOP; }

    private:
        std::vector<std::unique_ptr<transfer_coding>> m_codings_;
        std::function<void(const std::string&)> m_sink_;

        void _M_push(transfer_coding*);
    };

}}

#endif
