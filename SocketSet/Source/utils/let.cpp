/**
 * Created by TekuConcept on January 4, 2021
 */

#include "utils/let.h"

using namespace impact;


abstract_variable::abstract_variable() noexcept
: m_data(nullptr),
  m_sym("undefined")
{ }


bool
abstract_variable::truthy() const noexcept
{ return m_data != nullptr; }


let::let() noexcept
: m_var(nullptr)
{ }


let::let(const let& __other) noexcept
: m_var(__other.m_var)
{ }


let::let(let&& __other) noexcept
: m_var(std::move(__other.m_var))
{ }


let::let(std::shared_ptr<abstract_variable> __variable) noexcept
: m_var(__variable)
{ }


let::let(const char* __string)
: m_var(std::make_shared<impact_variable<std::string>>(__string))
{ }


let&
let::operator=(const let& __right) noexcept
{
    this->m_var = __right.m_var;
    return *this;
}


let&
let::operator=(let&& __right) noexcept
{
    this->m_var = std::move(__right.m_var);
    return *this;
}


let&
let::operator=(std::shared_ptr<abstract_variable> __right) noexcept
{
    this->m_var = __right;
    return *this;
}


let&
let::operator=(const char* __right)
{
    this->m_var = std::make_shared<impact_variable<std::string>>(__right);
    return *this;
}


let::operator bool() const noexcept
{ return this->truthy(); }


bool
let::truthy() const noexcept
{ return this->m_var && this->m_var->truthy(); }
