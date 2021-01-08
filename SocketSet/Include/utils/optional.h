/**
 * Created by TekuConcept on January 5, 2021
 */

#ifndef IMPACT_OPTIONAL_H
#define IMPACT_OPTIONAL_H

#include <optional>

namespace impact {

    template <typename T>
    class optional {
    public:
        // constexpr optional() noexcept;
        // constexpr optional(const optional& other);
        // constexpr optional(optional&& other);
        // template<class... Args>
        // constexpr explicit optional(Args&&... args);
        // ~optional() = default;

        // Member functions
        
        // destroys the contained value, if there is one
        // (public member function)
        // operator=
        
        // assigns contents
        // (public member function)
        // Observers
        // operator->
        // operator*
        
        // accesses the contained value
        // (public member function)
        // operator bool
        // has_value
        
        // checks whether the object contains a value
        // (public member function)
        // value
        
        // returns the contained value
        // (public member function)
        // value_or
        
        // returns the contained value if available, another value otherwise
        // (public member function)
        // Modifiers
        // swap
        
        // exchanges the contents
        // (public member function)
        // reset
        
        // destroys any contained value
        // (public member function)
        // emplace
        
        // constructs the contained value in-place
    };

} /* namespace impact */

#endif /* IMPACT_OPTIONAL_H */
