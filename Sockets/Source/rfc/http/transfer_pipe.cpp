/**
 * Created by TekuConcept on October 4, 2019
 */

#include "rfc/http/transfer_pipe.h"

using namespace impact;
using namespace http;


std::string transfer_pipe::EOP = "";


transfer_pipe::transfer_pipe()
{
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
}


void
transfer_pipe::set_codings(
    std::initializer_list<transfer_coding*> __codings)
{
    m_codings_.clear();
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
    for (auto it = __codings.begin(); it != __codings.end(); it++)
        _M_push(*it);
}


void
transfer_pipe::set_codings(
    std::vector<std::unique_ptr<transfer_coding>> __codings)
{
    m_codings_.clear();
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
    for (auto& coding : __codings)
        _M_push(coding.release());
}


void
transfer_pipe::_M_push(
    transfer_coding* __coding)
{
    if (__coding->name() == "chunked")
         m_codings_.back().reset(__coding);
    else m_codings_.insert(--m_codings_.end(),
            std::unique_ptr<transfer_coding>(__coding));
}


size_t
transfer_pipe::send(const std::string& __chunk)
{
    if (m_sink_ == nullptr) return 0;

    std::string buffer = __chunk;
    for (const auto& coding : m_codings_)
        buffer = coding->encode(buffer);
    m_sink_(buffer);

    // end of message
    // detach sink to prevent erroneous sends internally
    // must re-attach sink to write again
    if (__chunk.size() == 0)
        m_sink_ = nullptr;

    return __chunk.size();
}
