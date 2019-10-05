/**
 * Created by TekuConcept on October 4, 2019
 */

#ifndef _IMPACT_HTTP_TRANSFER_ENCODING_H_
#define _IMPACT_HTTP_TRANSFER_ENCODING_H_

#include <vector>
#include <memory>

#include "rfc/http/transfer_coding.h"

namespace impact {
namespace http {

    class transfer_encoding {
    public:
        enum class status { CONTINUE, DONE, ERROR };

        transfer_encoding();
        virtual ~transfer_encoding() = default;

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

        virtual status on_data_requested(std::string* buffer) = 0;

    private:
        std::vector<std::unique_ptr<transfer_coding>> m_codings_;

        void _M_push(transfer_coding*);
    };

}}

#endif
