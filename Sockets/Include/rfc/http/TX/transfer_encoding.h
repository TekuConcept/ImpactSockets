/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_TRANSFER_ENCODING_H_
#define _IMPACT_HTTP_TRANSFER_ENCODING_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "utils/case_string.h"
#include "rfc/http/TX/header_token.h"

namespace impact {
namespace http {
    
    class chunk_ext_token {
    public:
        chunk_ext_token(std::string name, std::string value="");
        ~chunk_ext_token();
        
    private:
        std::string m_name_;
        std::string m_value_;
    
    public:
        inline const std::string& name() const noexcept { return m_name_; }
        inline const std::string& value() const noexcept { return m_value_; }
    };
    
    
    class transfer_encoding {
    public:
        typedef std::function<std::string(const std::string&)> encoder_callback;
        typedef std::vector<chunk_ext_token> chunk_ext_list;
        typedef std::function<void(chunk_ext_list**)> ext_callback;
        typedef std::vector<header_token> header_list;
        typedef std::function<void(header_list**)> trailer_callback;
        
        transfer_encoding(std::string name, encoder_callback encoder);
        virtual ~transfer_encoding();
        
        static std::shared_ptr<transfer_encoding> chunked(
            ext_callback extension_callback   = nullptr,
            trailer_callback trailer_callback = nullptr
        );
    
    protected:
        encoder_callback m_encode_;
        transfer_encoding(std::string name);
        
    private:
        case_string m_name_;
        transfer_encoding();
    
    public:
        inline std::string encode(const std::string& data) const
        { return m_encode_(data); }
        inline const case_string& name() const noexcept
        { return m_name_; }
        
        friend class chunked_encoding;
    };
    typedef std::shared_ptr<transfer_encoding> transfer_encoding_ptr;
    
    
    class chunked_encoding : public transfer_encoding {
    public:
        chunked_encoding(
            ext_callback extension_callback   = nullptr,
            trailer_callback trailer_callback = nullptr);
        ~chunked_encoding();
    private:
        ext_callback     m_ext_callback_;
        trailer_callback m_trailer_callback_;
    };
}}

#endif
