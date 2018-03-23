/**
 * Created by TekuConcept on March 16, 2018
 */

#ifndef _INTERNAL_UTILS_H_
#define _INTERNAL_UTILS_H_

#include <future>
#include <chrono>

namespace Impact {
namespace Internal {
    template<typename R>
    bool is_ready(std::future<R> const& f) {
        return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }
}}

#endif