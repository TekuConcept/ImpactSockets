/**
 * Created by TekuConcept on October 14, 2019
 * 
 * Like a vector type but with a limited size
 */

#ifndef _IMPACT_UTILS_BATCH_H_
#define _IMPACT_UTILS_BATCH_H_

#include <vector>
#include <stdexcept>
#include <iterator>

namespace impact {

    template <typename T, size_t limit>
    class batch {
    public:
        batch() : m_drop_unused_(true) { }
        explicit batch( const std::allocator<T>& alloc )
        : m_data_(alloc), m_drop_unused_(true) { }
        batch(
            size_t count,
            const T& value,
            const std::allocator<T>& alloc = std::allocator<T>())
        : m_data_(count > limit ? limit : count, value, alloc),
          m_drop_unused_(true)
        { }
        explicit batch( size_t count )
        : m_data_(count > limit ? limit : count),
          m_drop_unused_(true)
        { }
        template< class InputIt >
        batch(
            InputIt first,
            InputIt last,
            const std::allocator<T>& alloc = std::allocator<T>())
        : m_data_(first, last, alloc),
          m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(const std::vector<T>& other)
        : m_data_(other), m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(const std::vector<T>& other, const std::allocator<T>& alloc)
        : m_data_(other, alloc), m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(std::vector<T>&& other)
        : m_data_(std::move(other)), m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(std::vector<T>&& other, const std::allocator<T>& alloc)
        : m_data_(std::move(other), alloc), m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(
            std::initializer_list<T> init,
            const std::allocator<T>& alloc = std::allocator<T>())
        : m_data_(init, alloc), m_drop_unused_(true)
        { if (m_data_.size() > limit) m_data_.resize(limit); }
        batch(const batch& other)
        : m_data_(other.m_data_),
          m_drop_unused_(other.m_drop_unused_)
        { }
        batch(batch&& other)
        : m_data_(std::move(other.m_data_)),
          m_drop_unused_(other.m_drop_unused_)
        { }

        inline void operator=(const batch& other)
        {
            this->m_data_ = other.m_data_;
            this->m_drop_unused_ = other.m_drop_unused_;
        }
        inline void operator=(const std::vector<T>& other)
        {
            this->m_data_ = other;
            this->m_drop_unused_ = true;
        }
        inline void operator=(batch&& other)
        {
            this->m_data_ = std::move(other.m_data_);
            this->m_drop_unused_ = other.m_drop_unused_;
        }
        inline void operator=(std::vector<T>&& other)
        {
            this->m_data_ = std::move(other);
            this->m_drop_unused_ = true;
        }

        // other
        inline bool& allow_dropping() { return m_drop_unused_; }

        // element access
        // std::out_of_range if !(pos < size())
        inline T& at( size_t pos ) { return m_data_.at(pos); };
        inline T& operator[]( size_t pos ) { return m_data_[pos]; }
        inline T& front() { return m_data_.front(); }
        inline T& back() { return m_data_.back(); }
        inline const std::vector<T>& raw() const { return m_data_; }

        // iterators
        inline typename std::vector<T>::const_iterator begin() noexcept
        { return std::move(m_data_.begin()); }
        inline typename std::vector<T>::const_iterator end() noexcept
        { return std::move(m_data_.end()); }
        inline typename std::vector<T>::const_reverse_iterator rbegin() noexcept
        { return std::move(m_data_.rbegin()); }
        inline typename std::vector<T>::const_reverse_iterator rend() noexcept
        { return std::move(m_data_.rend()); }
        inline typename std::vector<T>::const_iterator cbegin() const noexcept
        { return std::move(m_data_.cbegin()); }
        inline typename std::vector<T>::const_iterator cend() const noexcept
        { return std::move(m_data_.cend()); }
        inline typename std::vector<T>::const_reverse_iterator crbegin() const noexcept
        { return std::move(m_data_.crbegin()); }
        inline typename std::vector<T>::const_reverse_iterator crend() const noexcept
        { return std::move(m_data_.crend()); }

        // capacity
        inline size_t size() const noexcept { return m_data_.size(); }
        inline bool empty() const noexcept { return m_data_.empty(); }
        inline size_t max_size() const noexcept { return limit; }
        inline void reserve( size_t new_cap ) { m_data_.reserve(new_cap); }
        inline size_t capacity() const noexcept { return m_data_.capacity(); }
        inline void shrink_to_fit() { m_data_.shrink_to_fit(); }

        // modifiers
        inline void clear() noexcept { m_data_.clear(); }
        typename std::vector<T>::const_iterator insert(
            typename std::vector<T>::const_iterator pos,
            const batch& value)
        {
            if (m_data_.size() == limit) {
                if (m_drop_unused_) return pos;
                else throw std::length_error("batch is full");
            }
            else return m_data_.insert(pos, value);
        }
        typename std::vector<T>::const_iterator insert(
            typename std::vector<T>::const_iterator pos,
            T&& value)
        {
            if (m_data_.size() == limit) {
                if (m_drop_unused_) return pos;
                else throw std::length_error("batch is full");
            }
            else return m_data_.insert(pos, std::move(value));
        }
        typename std::vector<T>::const_iterator insert(
            typename std::vector<T>::const_iterator pos,
            typename std::vector<T>::const_iterator first,
            typename std::vector<T>::const_iterator last)
        {
            auto distance = std::distance(first, last);
            if ((distance + m_data_.size()) > limit) {
                if (m_drop_unused_)
                    last = first + (limit - m_data_.size());
                else throw std::length_error("not enough space");
            }
            return m_data_.insert(pos, first, last);
        }
        typename std::vector<T>::const_iterator insert(
            typename std::vector<T>::const_iterator pos,
            std::initializer_list<T> ilist)
        {
            if ((m_data_.size() + ilist.size()) > limit) {
                if (m_drop_unused_) {
                    auto begin = ilist.begin();
                    auto end = begin + (limit - m_data_.size());
                    return m_data_.insert(pos, begin, end);
                }
                else throw std::length_error("not enough space");
            }
            else return m_data_.insert(pos, ilist);
        }
        typename std::vector<T>::const_iterator erase(
            typename std::vector<T>::const_iterator pos)
        { return m_data_.erase(pos); }
        typename std::vector<T>::const_iterator erase(
            typename std::vector<T>::const_iterator first,
            typename std::vector<T>::const_iterator last)
        { return m_data_.erase(first, last); }
        void push_back( const T& value )
        {
            if (m_data_.size() == limit) {
                if (m_drop_unused_) return;
                else throw std::length_error("batch is full");
            }
            else m_data_.push_back(value);
        }
        void push_back( T&& value )
        {
            if (m_data_.size() == limit) {
                if (m_drop_unused_) return;
                else throw std::length_error("batch is full");
            }
            else m_data_.push_back(std::move(value));
        }
        inline void pop_back() { m_data_.pop_back(); }
        inline void resize(size_t count)
        { m_data_.resize(count > limit ? limit : count); }

    private:
        std::vector<T> m_data_;
        bool m_drop_unused_;
    };

}

#endif
