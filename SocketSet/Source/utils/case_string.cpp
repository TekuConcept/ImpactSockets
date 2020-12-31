/**
 * Created by TekuConcept on December 17, 2018
 */

#include "utils/case_string.h"

#include <algorithm>


bool
impact::operator==(
    const case_string& __lhs,
    const std::string& __rhs)
{
    if (__lhs.size() != __rhs.size()) return false;
    return std::equal(
        __lhs.begin(),
        __lhs.end(),
        __rhs.begin(),
        [] (char c, char d) -> bool {
            return std::tolower(c) == std::tolower(d);
        }
    );
}


bool
impact::operator==(
    const std::string& __lhs,
    const case_string& __rhs)
{
    if (__lhs.size() != __rhs.size()) return false;
    return std::equal(
        __lhs.begin(),
        __lhs.end(),
        __rhs.begin(),
        [] (char c, char d) -> bool {
            return std::tolower(c) == std::tolower(d);
        }
    );
}


bool
impact::operator!=(
    const case_string& __lhs,
    const std::string& __rhs)
{
    if (__lhs.size() != __rhs.size()) return true;
    return !std::equal(
        __lhs.begin(),
        __lhs.end(),
        __rhs.begin(),
        [] (char c, char d) -> bool {
            return std::tolower(c) == std::tolower(d);
        }
    );
}


bool
impact::operator!=(
    const std::string& __lhs,
    const case_string& __rhs)
{
    if (__lhs.size() != __rhs.size()) return true;
    return !std::equal(
        __lhs.begin(),
        __lhs.end(),
        __rhs.begin(),
        [] (char c, char d) -> bool {
            return std::tolower(c) == std::tolower(d);
        }
    );
}


std::ostream&
impact::operator<<(
    std::ostream&      __lhs,
    const case_string& __rhs)
{
    return __lhs.write(__rhs.data(), __rhs.size());
}


std::istream&
impact::operator>>(
    std::istream& __lhs,
    case_string&  __rhs)
{
    // requires coping of the string
    // would prefer move semantics or reading
    // directly into the rhs parameter
    // __rhs = case_string(temp.data(), temp.size());
    std::string temp;
    __lhs >> temp;
    swap(temp, __rhs);
    return __lhs;
}


impact::case_string&&
impact::move(std::string& __t)
{
    // hack
    return std::move(*(case_string*)&__t);
}


std::string&&
impact::move(case_string& __t)
{
    // hack
    return std::move(*(std::string*)&__t);
}


void
impact::swap(
    case_string& __lhs,
    std::string& __rhs)
{
    // hack
    std::swap(*(std::string*)(&__lhs), __rhs);
}


void
impact::swap(
    std::string& __lhs,
    case_string& __rhs)
{
    // hack
    std::swap(__lhs, *(std::string*)(&__rhs));
}
