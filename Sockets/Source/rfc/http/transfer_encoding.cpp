/**
 * Created by TekuConcept on October 4, 2019
 */

#include "rfc/http/transfer_encoding.h"

using namespace impact;
using namespace http;


transfer_encoding::transfer_encoding()
{
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
}


void
transfer_encoding::set_codings(
    std::initializer_list<transfer_coding*> __codings)
{
    m_codings_.clear();
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
    for (auto it = __codings.begin(); it != __codings.end(); it++)
        _M_push(*it);
}


void
transfer_encoding::set_codings(
    std::vector<std::unique_ptr<transfer_coding>> __codings)
{
    m_codings_.clear();
    m_codings_.push_back(
        std::unique_ptr<transfer_coding>(new chunked_coding()));
    for (auto& coding : __codings)
        _M_push(coding.release());
}


void
transfer_encoding::_M_push(
    transfer_coding* __coding)
{
    if (__coding->name() == "chunked")
         m_codings_.back().reset(__coding);
    else m_codings_.insert(--m_codings_.end(),
            std::unique_ptr<transfer_coding>(__coding));
}
