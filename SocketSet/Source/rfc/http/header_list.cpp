/**
 * Created by TekuConcept on October 3, 2019
 */

#include "rfc/http/header_list.h"

#include <algorithm>

#include "utils/impact_error.h"

using namespace impact;
using namespace http;


header_list::header_list()
: m_has_body_size_(false)
{}


header_list::header_list(std::initializer_list<header_t> __headers)
: m_has_body_size_(false)
{
    for (auto it = __headers.begin(); it != __headers.end(); it++)
        _M_insert(m_headers_.end(), *it);
}


header_list::header_list(const std::vector<header_t>& __headers)
: m_has_body_size_(false)
{
    for (const auto& header : __headers)
        _M_insert(m_headers_.end(), header);
}


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    const header_t&                       __value)
{ return _M_insert(__pos, __value); }


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    header_t&&                            __value)
{ return _M_insert(__pos, __value); }


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    std::vector<header_t>::const_iterator __first,
    std::vector<header_t>::const_iterator __last )
{
    bool merged;
    auto position = __pos;
    for (auto it = __first; it != __last; it++) {
        auto next_position = _M_insert(position, *it, &merged);
        if (!merged)
            position = ++next_position;
    }
    return position;
}


std::vector<header_t>::const_iterator
header_list::insert(
    std::vector<header_t>::const_iterator __pos,
    std::initializer_list<header_t>       __ilist )
{
    bool merged;
    auto position = __pos;
    for (auto it = __ilist.begin(); it != __ilist.end(); it++) {
        auto next_position = _M_insert(position, *it, &merged);
        if (!merged)
            position = ++next_position;
    }
    return position;
}


std::vector<header_t>::const_iterator
header_list::erase(
    std::vector<header_t>::const_iterator __pos )
{
    if (__pos != m_headers_.end() &&
        __pos->m_describes_body_size_)
        m_has_body_size_ = false;
    return m_headers_.erase(__pos);
}


std::vector<header_t>::const_iterator
header_list::erase(
    std::vector<header_t>::const_iterator __first,
    std::vector<header_t>::const_iterator __last )
{
    for (auto it = __first; it != __last; it++) {
        if (it->m_describes_body_size_) {
            m_has_body_size_ = false;
            break;
        }
    }
    return m_headers_.erase(__first, __last);
}


void
header_list::push_back(const header_t& __header)
{ _M_insert(m_headers_.end(), __header); }


void
header_list::push_back(header_t&& __header)
{ _M_insert(m_headers_.end(), __header); }


void
header_list::pop_back()
{
    if (m_headers_.size() == 0)
        m_has_body_size_ = false;
    else if (m_headers_.back().m_describes_body_size_)
        m_has_body_size_ = false;
    m_headers_.pop_back();
}


std::vector<header_t>::const_iterator
header_list::_M_insert(
    std::vector<header_t>::const_iterator __position,
    const header_t&                       __header,
    bool*                                 __merged)
{
    // - "Transfer-Encoding" and "Content-Length"
    // duplicates not allowed
    // - "Set-Cookie" duplicates allowed
    // - all other duplicates need to be concatenated

    if (m_has_body_size_ && __header.m_describes_body_size_)
        throw impact_error("cannot have both"
            " transfer-encoding and content-length headers"
            " nor duplicates thereof");
    else m_has_body_size_ |= __header.m_describes_body_size_;

    bool merged = false;
    auto result = std::find_if(
        m_headers_.begin(),
        m_headers_.end(),
        [&](const header_t& __token)
        { return __token.name() == __header.name(); }
    );

    if (result == m_headers_.end())
        return m_headers_.insert(__position, __header);
    else {
        if (__header.name() == "Set-Cookie")
            return m_headers_.insert(__position, __header);
        else {
            result->append_value_from(__header);
            merged = true;
        }
    }

    if (__merged) *__merged = merged;
    return __position;
}
