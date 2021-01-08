/**
 * Created by TekuConcept on January 19, 2020
 */

#ifndef IMPACT_LET_H
#define IMPACT_LET_H

#include <iostream>
#include <string>
#include <type_traits>
#include <memory>
#include "utils/impact_error.h"

namespace impact {

    #if defined(__llvm__) || defined(__clang__)
        #define __CLANG__
        // clang consistently defines the type name
        // both with and without rtti support
        // with-rtti: std::vector<int, std::allocator<int> >
        // no-rtti:   std::vector<int, std::allocator<int> >
        #if __has_feature(cxx_rtti)
            #include <cxxabi.h>
            #define RTTI_ENABLED
            #define TYPENAME(type) abi::__cxa_demangle(typeid((type)).name(), NULL, NULL, NULL)
            // clang-users install: libc++abi-dev
        #endif
    #elif defined(__GNUG__)
        #define __GCC__
        // GCC is not consistent in the type definition name
        // with-rtti: std::vector<int, std::allocator<int> >
        // no-rtti:   std::vector<int>
        #if defined(__GXX_RTTI)
            #include <cxxabi.h>
            #define RTTI_ENABLED
            #define TYPENAME(type) abi::__cxa_demangle(typeid((type)).name(), NULL, NULL, NULL)
        #endif
    #elif defined(_MSC_VER)
        #define __MSVC__
        // MSVC consistently defines the type name
        // both with and without rtti
        // with-rtti: std::vector<int, std::allocator<int> >
        // no-rtti:   std::vector<int, std::allocator<int> >
        #if defined(_CPPRTTI)
            #define RTTI_ENABLED
            #define TYPENAME(type) [&]() -> std::string {                   \
                const std::string prefix = "class ";                        \
                std::string name = typeid((type)).name();                   \
                size_t pos = 0;                                             \
                if (name.rfind(prefix, 0) == 0)                             \
                    name = name.substr(prefix.size());                      \
                while ((pos = name.find(prefix, pos)) != std::string::npos) \
                    name.replace(pos, prefix.size(), " ");                  \
                return name;                                                \
            }()
        #endif
    #endif

    class abstract_variable {
    public:
        abstract_variable();
        virtual ~abstract_variable() = default;
        virtual bool truthy() const;
        std::string class_name() const { return m_sym; }
        template <typename T>
        inline T& get() const { return *reinterpret_cast<T*>(m_data); }

     protected:
        void*       m_data;
        std::string m_sym;
        friend class let;
    };


    template <typename T>
    class impact_variable : public abstract_variable {
    public:
        impact_variable();
        impact_variable(T value);
        impact_variable(const impact_variable& right);
        impact_variable(impact_variable&& right);
        template <typename U = T, typename std::enable_if<
            !(std::is_reference<U>::value || std::is_fundamental<U>::value)
        >::type* = nullptr>
        impact_variable(T&& __value) : m_value(std::move(__value))
        { _M_base_init(); }
        virtual ~impact_variable() = default;

        impact_variable& operator=(const impact_variable& right);
        impact_variable& operator=(impact_variable&& right);
        inline T& value() { return m_value; }
        bool truthy() const override;

        static std::shared_ptr<impact_variable> create()
        { return std::make_shared<impact_variable>(); }
        static std::shared_ptr<impact_variable> create(const T& value)
        { return std::make_shared<impact_variable>(value); }
        template <typename U = T, typename std::enable_if<
            !(std::is_reference<U>::value || std::is_fundamental<U>::value)
        >::type* = nullptr>
        static std::shared_ptr<impact_variable> create(T&& value)
        { return std::make_shared<impact_variable>(std::move(value)); }

    protected:
        T m_value;

    private:
        void _M_base_init();
        bool _M_number_object_truthy(std::true_type) const
        { return ( this->m_value != 0 ); }
        bool _M_number_object_truthy(std::false_type) const
        { return abstract_variable::truthy(); }
        static void _M_private_class_name(std::string* __name) {
            // #if defined(RTTI_ENABLED)
            //     T type;
            //     *__name = TYPENAME(type);
            // #else // - hack to support non-rtti builds -
                #if defined(__CLANG__)
                    /*
                    static void impact::impact_variable<T>::_M_private_class_name(
                        std::string *) [T = T]
                    */
                    std::string name(__PRETTY_FUNCTION__);
                    auto index1 = name.find_first_of("[");
                    index1 += 5; // "[T = "
                    auto index2 = name.find_first_of(";]", index1);
                    *__name = name.substr(index1, index2 - index1);
                #elif defined(__GCC__)
                    /*
                    static void impact::impact_variable<T>::_M_private_class_name(
                        std::__cxx11::string*) [with T = T; std::__cxx11::string =
                        std::__cxx11::basic_string<char>]
                    */
                    std::string name(__PRETTY_FUNCTION__);
                    std::string prefix("static void impact::impact_variable<T>::_M_private_class_name(std::__cxx11::string*) [with T = ");
                    auto index = name.find_first_of(";]", prefix.length());
                    *__name = name.substr(prefix.length(), index - prefix.length());
                #elif defined(__MSVC__)
                    // impact_variable<class T>::class_name
                    // impact_variable<primitive>::class_name
                    size_t pos = 0;
                    const std::string prefix = "class ";
                    std::string name(__FUNCTION__);
                    auto first = name.find_first_of("<") + 1;
                    auto last  = name.find_last_of(">");
                    auto skip  = name.find_first_of(prefix, first);
                    if (skip == first) first += prefix.size();
                    name = name.substr(first, last - first);
                    while ((pos = name.find(prefix, pos)) != std::string::npos)
                        name.replace(pos, prefix.size(), " ");
                    *__name = name;
                #else /* unsupported compiler */
                    *__name = "[feature not supported]";
                #endif
                // std::cout << "> " << *__name << "\n";
            // #endif /* RTTI_ENABLED */
        }
    };


    class let {
    public:
        let();
        let(const let& other);
        let(let&& other);
        let(std::shared_ptr<abstract_variable> variable);
        let(const char* value);
        template <typename T>
        let(std::shared_ptr<impact_variable<T>> variable)
        : m_var(variable) { }
        template <typename T>
        let(T variable)
        : m_var(std::make_shared<impact_variable<T>>(variable)) { }
        ~let() = default;

        let& operator=(const let& right);
        let& operator=(let&& right);
        let& operator=(std::shared_ptr<abstract_variable> right);
        let& operator=(const char* right);
        template <typename T>
        let& operator=(std::shared_ptr<impact_variable<T>> right) {
            this->m_var = right;
            return *this;
        }
        template <typename T>
        let& operator=(T right) {
            this->m_var = std::make_shared<impact_variable<T>>(right);
            return *this;
        }
        operator bool() const;

        template <typename T>
        inline T& get() const
        {
            if (m_var == nullptr) throw impact_error("undefined variable");
            return *reinterpret_cast<T*>(m_var->m_data);
        }
        bool truthy() const;
        inline std::string class_name() const
        { return (m_var != nullptr) ? m_var->class_name() : "undefined"; }
        inline bool is_undefined() const
        { return m_var == nullptr; }

    private:
        std::shared_ptr<abstract_variable> m_var;
    };


    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
    * Function Implementations
    \* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


    template <typename T>
    impact_variable<T>::impact_variable()
    { _M_base_init(); }


    template <typename T>
    impact_variable<T>::impact_variable(T __value)
    : m_value(__value)
    { _M_base_init(); }


    template <typename T>
    impact_variable<T>::impact_variable(const impact_variable<T>& __right)
    : m_value(__right.m_value)
    {
        abstract_variable::m_data = &this->m_value;
        abstract_variable::m_sym  = __right.m_sym;
    }


    template <typename T>
    impact_variable<T>::impact_variable(impact_variable<T>&& __right)
    : m_value(std::move(__right.m_value))
    {
        abstract_variable::m_data = &this->m_value;
        abstract_variable::m_sym  = __right.m_sym;
    }


    template <typename T>
    impact_variable<T>&
    impact_variable<T>::operator=(const impact_variable<T>& __right)
    {
        this->m_value = __right.m_value;
        return *this;
    }


    template <typename T>
    impact_variable<T>&
    impact_variable<T>::operator=(impact_variable<T>&& __right)
    {
        this->m_value = std::move(__right.m_value);
        return *this;
    }


    template <typename T>
    bool
    impact_variable<T>::truthy() const
    {
        typedef typename std::remove_cv<T>::type simple_type;
        return (std::is_same<std::string, simple_type>::value &&
            ((std::string*)&this->m_value)->length() > 0) ||
            _M_number_object_truthy(std::is_arithmetic<T>());
    }


    template <typename T>
    void
    impact_variable<T>::_M_base_init()
    {
        typedef typename std::remove_cv<T>::type simple_type;
        abstract_variable::m_data = &this->m_value;
        if (std::is_same<std::string, simple_type>::value)
            abstract_variable::m_sym = "std::string";
        else {
            std::string ref;
            _M_private_class_name(&ref);
            abstract_variable::m_sym = ref;
        }
    }


} /* namespace impact */

#endif /* IMPACT_LET_H */
