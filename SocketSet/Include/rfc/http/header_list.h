/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef _IMPACT_HTTP_HEADER_LIST_H_
#define _IMPACT_HTTP_HEADER_LIST_H_

#include <vector>

#include "rfc/http/header.h"

namespace impact {
namespace http {

    // vector proxy class to ensure proper behavior
    class header_list {
    public:
        header_list();
        header_list(std::initializer_list<header_t> headers);
        header_list(const std::vector<header_t>& headers);
        ~header_list() = default;

        // element access
        // std::out_of_range if !(pos < size())
        inline const header_t& at( size_t pos )
        { return m_headers_.at(pos); };
        inline const header_t& operator[]( size_t pos ) const
        { return m_headers_[pos]; }
        inline const header_t& front() const
        { return m_headers_.front(); }
        inline const header_t& back() const
        { return m_headers_.back(); }

        // iterators
        inline std::vector<header_t>::const_iterator begin() const noexcept
        { return std::move(m_headers_.cbegin()); }
        inline std::vector<header_t>::const_iterator end() const noexcept
        { return std::move(m_headers_.cend()); }
        inline std::vector<header_t>::const_reverse_iterator rbegin() const noexcept
        { return std::move(m_headers_.crbegin()); }
        inline std::vector<header_t>::const_reverse_iterator rend() const noexcept
        { return std::move(m_headers_.crend()); }
        inline size_t size() const noexcept
        { return m_headers_.size(); }

        // capacity
        inline bool empty() const noexcept
        { return m_headers_.empty(); }
        inline size_t max_size() const noexcept
        { return m_headers_.max_size(); }
        inline void reserve( size_t new_cap )
        { m_headers_.reserve(new_cap); }
        inline size_t capacity() const noexcept
        { return m_headers_.capacity(); }
        inline void shrink_to_fit()
        { m_headers_.shrink_to_fit(); }

        // modifiers
        inline void clear() noexcept
        { m_headers_.clear(); }
        std::vector<header_t>::const_iterator insert(
            std::vector<header_t>::const_iterator pos, const header_t& value );
        std::vector<header_t>::const_iterator insert(
            std::vector<header_t>::const_iterator pos, header_t&& value );
        std::vector<header_t>::const_iterator insert(
            std::vector<header_t>::const_iterator pos,
            std::vector<header_t>::const_iterator first,
            std::vector<header_t>::const_iterator last );
        std::vector<header_t>::const_iterator insert(
            std::vector<header_t>::const_iterator pos,
            std::initializer_list<header_t> ilist );
        std::vector<header_t>::const_iterator erase(
            std::vector<header_t>::const_iterator pos );
        std::vector<header_t>::const_iterator erase(
            std::vector<header_t>::const_iterator first,
            std::vector<header_t>::const_iterator last );
        void push_back( const header_t& value );
        void push_back( header_t&& value );
        void pop_back();

    private:
        std::vector<header_t> m_headers_;
        bool m_has_body_size_;

        std::vector<header_t>::const_iterator _M_insert(
            std::vector<header_t>::const_iterator,
            const header_t&, bool* = NULL);

        friend class message_t;
    };

}}

#endif
