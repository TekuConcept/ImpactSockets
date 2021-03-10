/**
 * Created by TekuConcept on March 8, 2021
 */

#define IMPACT_UTILS_RING_BUFFER_H /* prototype feature - disable for now */
#ifndef IMPACT_UTILS_RING_BUFFER_H
#define IMPACT_UTILS_RING_BUFFER_H

#include <vector>
#include <algorithm>
#include <stdexcept>

namespace impact {

    template <typename T, typename Allocator = std::allocator<T>>
    class ring_buffer {
    private:
        enum class running_state_t { NORMAL, UNDERRUN, OVERRUN };

    public:
        typedef std::vector<T, Allocator> buffer_type;

        typedef buffer_type::reference       reference;
        typedef buffer_type::const_reference const_reference;
        typedef buffer_type::allocator_type  allocator_type;
        typedef buffer_type::size_type       size_type;

        // class : public std::iterator<std::bidirectional_iterator_tag, T>
        struct _iterator_impl_ {
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type   = buffer_type::difference_type;
            using value_type        = buffer_type::value_type;
            using pointer           = buffer_type::pointer;
            using reference         = buffer_type::reference;

            _iterator_impl_(pointer ptr, size_type size, size_type index)
            : m_ptr(ptr), m_size(size), m_index(index) {}

            reference operator*() const { return m_ptr[m_index]; }
            pointer operator->() { return &m_ptr[m_index]; }

            _iterator_impl_& operator++()
            { m_index = (m_index + 1) % m_size; return *this; }
            _iterator_impl_ operator++(int)
            { _iterator_impl_ tmp = *this; ++(*this); return tmp; }
            _iterator_impl_& operator--()
            { m_index = (m_size + m_index - 1) % m_size; return *this; }
            _iterator_impl_ operator--(int)
            { _iterator_impl_ tmp = *this; --(*this); return tmp; }

            friend bool operator== (const _iterator_impl_& a, const _iterator_impl_& b)
            { return a.m_ptr[a.m_index] == b.m_ptr[b.m_index]; };
            friend bool operator!= (const _iterator_impl_& a, const _iterator_impl_& b)
            { return a.m_ptr[a.m_index] != b.m_ptr[b.m_index]; };

            inline size_type index() const
            { return m_index; }

        private:
            pointer m_ptr;
            size_type m_size;
            size_type m_index;
        };

        typedef _iterator_impl_ iterator;
        typedef const _iterator_impl_ const_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;

        //
        // constructors
        //

        ring_buffer()
        : m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(true)
        { }
        explicit ring_buffer(const Allocator& alloc)
        : m_base(alloc),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(true)
        { }
        explicit ring_buffer(
            size_type count,
            const T& value = T(),
            const Allocator& alloc = Allocator())
        : m_base(count, value, alloc),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(count == 0)
        { }
        explicit ring_buffer(size_type count)
        : m_base(count),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(count == 0)
        { }
        explicit ring_buffer(
            size_type count,
            const Allocator& alloc = Allocator())
        : m_base(count, alloc),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(count == 0)
        { }
        template <class InputIt>
        ring_buffer(
            InputIt first,
            InputIt last,
            const Allocator& alloc = Allocator())
        : m_base(first, last, alloc),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(m_base.size() == 0)
        { }
        ring_buffer(const ring_buffer& other)
        : m_base(other.m_base),
          m_start_index(other.m_start_index),
          m_end_index(other.m_end_index),
          m_running_state(other.m_running_state),
          m_full(other.m_full)
        { }
        ring_buffer(
            const ring_buffer& other,
            const Allocator& alloc)
        : m_base(other.m_base, alloc),
          m_start_index(other.m_start_index),
          m_end_index(other.m_end_index),
          m_running_state(other.m_running_state),
          m_full(other.m_full)
        { }
        ring_buffer(ring_buffer&& other)
        : m_base(std::move(other.m_base)),
          m_start_index(other.m_start_index),
          m_end_index(other.m_end_index),
          m_running_state(other.m_running_state),
          m_full(other.m_full)
        { }
        ring_buffer(
            ring_buffer&& other,
            const Allocator& alloc)
        : m_base(std::move(other.m_base), alloc),
          m_start_index(other.m_start_index),
          m_end_index(other.m_end_index),
          m_running_state(other.m_running_state),
          m_full(other.m_full)
        { }
        ring_buffer(
            std::initializer_list<T> init,
            const Allocator& alloc = Allocator())
        : m_base(init, alloc),
          m_start_index(0),
          m_end_index(0),
          m_running_state(running_state_t::NORMAL),
          m_full(m_base.size() == 0)
        { }

        //
        // destructor
        //

        ~ring_buffer() = default;

        //
        // assignment
        //

        ring_buffer& operator=(const ring_buffer& other)
        {
            m_base          = other.m_base;
            m_start_index   = other.m_start_index;
            m_end_index     = other.m_end_index;
            m_running_state = other.m_running_state;
            m_full          = other.m_full;
            return *this;
        }
        ring_buffer& operator=(ring_buffer&& other)
        {
            m_base          = std::move(other.m_base);
            m_start_index   = other.m_start_index;
            m_end_index     = other.m_end_index;
            m_running_state = other.m_running_state;
            m_full          = other.m_full;
            return *this;
        }
        ring_buffer& operator=(std::initializer_list<T> ilist)
        {
            m_base          = ilist;
            m_start_index   = 0;
            m_end_index     = 0;
            m_running_state = running_state_t::NORMAL;
            m_full          = m_base.size() == 0;
            return *this;
        }

        void assign(
            size_type count,
            const T& value)
        {
            m_base.assign(count, value);
            m_start_index   = 0;
            m_end_index     = 0;
            m_running_state = running_state_t::NORMAL;
            m_full          = count == 0;
        }
        template< class InputIt >
        void assign(
            InputIt first,
            InputIt last)
        {
            m_base.assign(first, last);
            m_start_index   = 0;
            m_end_index     = 0;
            m_running_state = running_state_t::NORMAL;
            m_full          = m_base.size() == 0;
        }
        void assign(std::initializer_list<T> ilist)
        {
            m_base.assign(ilist);
            m_start_index   = 0;
            m_end_index     = 0;
            m_running_state = running_state_t::NORMAL;
            m_full          = m_base.size() == 0;
        }

        allocator_type get_allocator() const noexcept
        { return m_base.get_allocator(); }

        //
        // element access
        //

        reference at(size_type pos) {
            if (pos > m_base.size())
                throw std::out_of_range();
            return (*this)[pos];
        }
        const_reference at(size_type pos) const
        {
            if (pos > m_base.size())
                throw std::out_of_range();
            return (*this)[pos];
        }
        reference operator[](size_type pos)
        { return m_base[(m_start_index + pos) & m_base.size()]; }
        const_reference operator[](size_type pos) const
        { return m_base[(m_start_index + pos) & m_base.size()]; }
        reference front()
        { return m_base[m_start_index]; }
        const_reference front() const
        { return m_base[m_start_index]; }
        reference back()
        { return m_base[m_end_index]; }
        const_reference back() const
        { return m_base[m_end_index]; }
        T* data() noexcept
        { return m_base.data(); }
        const T* data() const noexcept
        { return m_base.data(); }

        //
        // iterators
        //

        iterator begin() noexcept
        { return iterator(m_base.data(), m_base.size(), m_start_index); }
        const_iterator begin() const noexcept
        { return iterator(m_base.data(), m_base.size(), m_start_index); }
        const_iterator cbegin() const noexcept
        { return iterator(m_base.data(), m_base.size(), m_start_index); }
        iterator end() noexcept
        { return iterator(m_base.data(), m_base.size(), m_end_index); }
        const_iterator end() const noexcept
        { return iterator(m_base.data(), m_base.size(), m_end_index); }
        const_iterator cend() const noexcept
        { return iterator(m_base.data(), m_base.size(), m_end_index); }
        reverse_iterator rbegin() noexcept
        { return reverse_iterator(begin()); }
        const_reverse_iterator rbegin() const noexcept
        { return const_reverse_iterator(cbegin()); }
        const_reverse_iterator crbegin() const noexcept
        { return const_reverse_iterator(cbegin()); }
        reverse_iterator rend() noexcept
        { return reverse_iterator(end()); }
        const_reverse_iterator rend() const noexcept
        { return const_reverse_iterator(cend()); }
        const_reverse_iterator crend() const noexcept
        { return const_reverse_iterator(cend()); }

        //
        // capacity
        //

        bool empty() const noexcept
        { return !m_full && (m_start_index == m_end_index); }
        bool full() const noexcept
        { return m_full; }
        bool overrun() const noexcept
        { return m_running_state == running_state_t::OVERRUN; }
        bool underrun() const noexcept
        { return m_running_state == running_state_t::UNDERRUN; }
        size_type size() const noexcept
        {
            if (m_full) return m_base.size();
            if (m_end_index >= m_start_index)
                return (m_end_index - m_start_index);
            else return (m_base.size() + m_end_index - m_start_index);
        }
        size_type max_size() const noexcept
        { return m_base.max_size(); }
        size_type capacity() const noexcept
        { return m_base.size(); }
        inline size_type start_index() const noexcept
        { return m_start_index; }
        inline size_type end_index() const noexcept
        { return m_end_index; }

        //
        // modifiers
        //

        void clear() noexcept
        {
            m_start_index   = 0;
            m_end_index     = 0;
            m_running_state = running_state_t::NORMAL;
            std::fill(m_base.begin(), m_base.end(), T());
        }
        iterator insert(
            const_iterator pos,
            const T& value)
        {
            T temp     = value;
            auto index = pos.index();
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_increment_index(m_start_index);
            }
            _M_increment_index(m_end_index);
            m_full = m_start_index == m_end_index;
            do {
                std::swap(m_base[index], temp);
                _M_increment_index(index);
            } while (index ! m_end_index);
            return pos;
        }
        iterator insert(
            const_iterator pos,
            T&& value)
        {
            T temp     = std::move(value);
            auto index = pos.index();
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_increment_index(m_start_index);
            }
            _M_increment_index(m_end_index);
            m_full = m_start_index == m_end_index;
            do {
                std::swap(m_base[index], temp);
                _M_increment_index(index);
            } while (index ! m_end_index);
            return pos;
        }
        iterator insert(
            const_iterator pos,
            size_type count,
            const T& value)
        {
            if (count > m_base.size()) {
                m_start_index   = 0;
                m_end_index     = 0;
                m_running_state = running_state_t::OVERRUN;
                m_full          = true;
                std::fill(m_base.begin(), m_base.end(), value);
                return begin();
            }
            size_type current_size = size();
            size_type base_size    = m_base.size();
            size_type insert_index = pos.index();
            if (count + current_size > base_size)
                m_running_state = running_state_t::OVERRUN;
            _M_decrement_index(insert_index, m_start_index);
            _M_realign_memory();
            m_base.insert(m_base.begin() + insert_index, count, value);
            if (m_running_state == running_state_t::OVERRUN) {
                size_type remaining   = base_size - current_size;
                size_type overwritten = count - remaining;
                size_type left_sift   = std::min(overwritten, insert_index);
                std::rotate(
                    m_base.begin(),
                    m_base.begin() + left_shift,
                    m_base.end());
                _M_decrement_index(insert_index, left_sift);
                m_start_index = 0;
                m_end_index   = 0;
            }
            else _M_increment_index(m_end_index, count);
            m_full = m_start_index == m_end_index;
            m_base.resize(base_size);
            return iterator(m_base.data(), m_base.size(), insert_index);
        }
        template <class InputIt>
        iterator insert(
            const_iterator pos,
            InputIt first,
            InputIt last)
        {
            size_type current_size = size();
            size_type base_size    = m_base.size();
            size_type insert_index = pos.index();
            _M_decrement_index(insert_index, m_start_index);
            _M_realign_memory();
            m_base.insert(m_base.begin() + insert_index, first, last);
            size_type count = m_base.size() - base_size;
            if (count + current_size > base_size) {
                m_running_state = running_state_t::OVERRUN;
                size_type remaining   = base_size - current_size;
                size_type overwritten = count - remaining;
                size_type left_sift   = std::min(overwritten, insert_index);
                std::rotate(
                    m_base.begin(),
                    m_base.begin() + left_shift,
                    m_base.end());
                _M_decrement_index(insert_index, left_sift);
                m_start_index = 0;
                m_end_index   = 0;
            }
            else _M_increment_index(m_end_index, count);
            m_full = m_start_index == m_end_index;
            m_base.resize(base_size);
            return iterator(m_base.data(), m_base.size(), insert_index);
        }
        iterator insert(
            const_iterator pos,
            std::initializer_list<T> ilist)
        { return insert(pos, ilist.begin(), ilist.end()); }
        template <class... Args>
        iterator emplace(const_iterator pos, Args&&... args)
        {
            T temp(args);
            auto index = pos.index();
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_increment_index(m_start_index);
            }
            _M_increment_index(m_end_index);
            m_full = m_start_index == m_end_index;
            do {
                std::swap(m_base[index], temp);
                _M_increment_index(index);
            } while (index != m_end_index);
            return pos;
        }
        template <class... Args>
        void emplace_back(Args&&... args)
        { emplace(cend(), std::forward(args)); }
        iterator erase(const_iterator pos)
        {
            size_type index = pos.index();
            m_running_state = running_state_t::NORMAL;
            if (empty()) return pos;
            if (index == m_end_index) {
                pop_back();
                return iterator(m_base.data(), m_base.size(), m_end_index);
            }
            if (index == m_start_index) {
                pop_front();
                return iterator(m_base.data(), m_base.size(), m_start_index);
            }
            size_type next_index = index;
            _M_increment_index(next_index);
            while (next_index != m_end_index) {
                m_base[index] = std::move(m_base[next_index]);
                index = next_index;
                _M_increment_index(next_index);
            }
            m_end_index = index;
            m_full = m_start_index == m_end_index;
            return pos;
        }
        iterator erase(const_iterator first, const_iterator last)
        {
            size_type start = first.index();
            size_type end   = last.index();
            if (start == end) {
                if (m_full) {
                    m_full          = false;
                    m_start_index   = 0;
                    m_end_index     = 0;
                    m_running_state = running_state_t::NORMAL;
                    return end();
                }
                else return first;
            }
            else m_full = false;
            _M_decrement_index(start, m_start_index);
            _M_decrement_index(end, m_start_index);
            _M_realign_memory();
            if (start > end) std::swap(start, end);
            m_running_state = running_state_t::NORMAL;
            if (end = m_end_index) {
                m_end_index = start;
                return iterator(m_base.data(), m_base.size(), m_end_index);
            }
            if (start = m_start_index) {
                m_start_index = end;
                return iterator(m_base.data(), m_base.size(), m_start_index);
            }
            iterator result(m_base.data(), m_base.size(), end);
            do {
                std::swap(m_base[start], m_base[end]);
                // Note: memory-aligned
                start++;
                end++;
            } while (end != m_end_index);
            return result;
        }
        void push_back(const T& value)
        {
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_increment_index(m_start_index);
            }
            m_base[m_end_index] = value;
            _M_increment_index(m_end_index);
            m_full = m_start_index == m_end_index;
        }
        void push_back(T&& value)
        {
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_increment_index(m_start_index);
            }
            m_base[m_end_index] = value;
            _M_increment_index(m_end_index);
            m_full = m_start_index == m_end_index;
        }
        void pop_back()
        {
            if (empty())
                m_running_state = running_state_t::UNDERRUN;
            else _M_decrement_index(m_end_index);
        }
        void push_front(const T& value)
        {
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_decrement_index(m_end_index);
            }
            _M_decrement_index(m_start_index);
            m_full = m_start_index == m_end_index;
            m_base[m_start_index] = value
        }
        void push_front(T&& value)
        {
            if (m_full) {
                m_running_state = running_state_t::OVERRUN;
                _M_decrement_index(m_end_index);
            }
            _M_decrement_index(m_start_index);
            m_full = m_start_index == m_end_index;
            m_base[m_start_index] = std::move(value);
        }
        void pop_front()
        {
            if (empty())
                m_running_state = running_state_t::UNDERRUN;
            else _M_increment_index(m_start_index);
        }
        void resize(size_type count)
        {
            _M_realign_memory();
            if (count < size())
                m_running_state = running_state_t::OVERRUN;
            else m_running_state = running_state_t::NORMAL;
            m_base.resize(count);
        }
        void resize(size_type count, const value_type& value)
        {
            _M_realign_memory();
            if (count < size())
                m_running_state = running_state_t::OVERRUN;
            else m_running_state = running_state_t::NORMAL;
            m_base.resize(count, value);
        }
        void swap(ring_buffer& other)
        {
            std::swap(m_base, other.m_base);
            std::swap(m_start_index, other.m_start_index);
            std::swap(m_end_index, other.m_end_index);
            std::swap(m_running_state, other.m_running_state);
            std::swap(m_full, other.m_full);
        }

    private:
        buffer_type     m_base;
        size_type       m_start_index;
        size_type       m_end_index;
        running_state_t m_running_state;
        bool            m_full;

        inline void _M_increment_index(
            size_type index,
            size_type count = 1)
        { index = (index + count) % m_base.size(); }
        inline void _M_decrement_index(
            size_type index,
            size_type count = 1)
        { index = (m_base + index - count) % m_base.size(); }
        void _M_realign_memory() {
            if (m_start_index != 0) {
                std::rotate(
                    m_base.begin(),
                    m_base.begin() + m_start_index,
                    m_base.end());
                _M_decrement_index(m_end_index, m_start_index);
                m_start_index = 0;
            }
        }

        friend inline bool
        operator==(
            const ring_buffer<T,Alloc>& lhs,
            const ring_buffer<T,Alloc>& rhs);
    };

    //
    // non-member functions
    //

    template <typename T, typename Alloc>
    inline bool
    operator==(
        const ring_buffer<T,Alloc>& lhs,
        const ring_buffer<T,Alloc>& rhs)
    {
        return lhs.size() == rhs.size() &&
            std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

} /* namespace impact */


namespace std {
    //
    // specialization for impact::ring_buffer
    //
    template <typename T, typename Alloc>
    inline void
    swap(
        impact::ring_buffer<T,Alloc>& lhs,
        impact::ring_buffer<T,Alloc>& rhs)
    { lhs.swap(rhs); }
} /* namespace std */

#endif /* IMPACT_UTILS_RING_BUFFER_H */
