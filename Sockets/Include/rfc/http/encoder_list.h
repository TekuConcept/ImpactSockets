/**
 * Created by TekuConcept on October 3, 2019
 */

#ifndef _IMPACT_HTTP_ENCODER_LIST_H_
#define _IMPACT_HTTP_ENCODER_LIST_H_

#include <vector>
#include <memory>

namespace impact {
namespace http {

    // vector proxy class to ensure proper behavior
    // class encoder_t;
    // class encoder_list {
    // public:
    //     typedef std::unique_ptr<encoder_t> encoder_ptr;

    //     encoder_list();
    //     encoder_list(std::initializer_list<encoder_ptr> encoders);
    //     encoder_list(const std::vector<encoder_ptr>& encoders);
    //     ~encoder_list() = default;

    //     // element access
    //     // std::out_of_range if !(pos < size())
    //     inline const encoder_ptr& at( size_t pos )
    //     { return m_encoders_.at(pos); };
    //     inline const encoder_ptr& operator[]( size_t pos ) const
    //     { return m_encoders_[pos]; }
    //     inline const encoder_ptr& front() const
    //     { return m_encoders_.front(); }
    //     inline const encoder_ptr& back() const
    //     { return m_encoders_.back(); }

    //     // iterators
    //     inline std::vector<encoder_ptr>::const_iterator begin() const noexcept
    //     { return std::move(m_encoders_.cbegin()); }
    //     inline std::vector<encoder_ptr>::const_iterator end() const noexcept
    //     { return std::move(m_encoders_.cend()); }
    //     inline std::vector<encoder_ptr>::const_reverse_iterator rbegin() const noexcept
    //     { return std::move(m_encoders_.crbegin()); }
    //     inline std::vector<encoder_ptr>::const_reverse_iterator rend() const noexcept
    //     { return std::move(m_encoders_.crend()); }
    //     inline size_t size() const noexcept
    //     { return m_encoders_.size(); }

    //     // capacity
    //     inline bool empty() const noexcept
    //     { return m_encoders_.empty(); }
    //     inline size_t max_size() const noexcept
    //     { return m_encoders_.max_size(); }
    //     inline void reserve( size_t new_cap )
    //     { m_encoders_.reserve(new_cap); }
    //     inline size_t capacity() const noexcept
    //     { return m_encoders_.capacity(); }
    //     inline void shrink_to_fit()
    //     { m_encoders_.shrink_to_fit(); }

    //     // modifiers
    //     inline void clear() noexcept
    //     { m_encoders_.clear(); }
    //     std::vector<encoder_ptr>::const_iterator insert(
    //         std::vector<encoder_ptr>::const_iterator pos, encoder_ptr&& value );
    //     std::vector<encoder_ptr>::const_iterator insert(
    //         std::vector<encoder_ptr>::const_iterator pos,
    //         std::vector<encoder_ptr>::const_iterator first,
    //         std::vector<encoder_ptr>::const_iterator last );
    //     std::vector<encoder_ptr>::const_iterator insert(
    //         std::vector<encoder_ptr>::const_iterator pos,
    //         std::initializer_list<encoder_ptr> ilist );
    //     std::vector<encoder_ptr>::const_iterator erase(
    //         std::vector<encoder_ptr>::const_iterator pos );
    //     std::vector<encoder_ptr>::const_iterator erase(
    //         std::vector<encoder_ptr>::const_iterator first,
    //         std::vector<encoder_ptr>::const_iterator last );
    //     void push_back( encoder_ptr&& value );
    //     void pop_back();

    // private:
    //     std::vector<encoder_ptr> m_encoders_;

    //     std::vector<encoder_ptr>::const_iterator _M_insert(
    //         std::vector<encoder_ptr>::const_iterator,
    //         encoder_ptr, bool* = NULL);

    //     friend class message_t;
    // };

}}

#endif
