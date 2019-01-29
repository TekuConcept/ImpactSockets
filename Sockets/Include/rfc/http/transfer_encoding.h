/**
 * Created by TekuConcept on December 28, 2018
 */

#ifndef _IMPACT_HTTP_TRANSFER_ENCODING_H_
#define _IMPACT_HTTP_TRANSFER_ENCODING_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "utils/cloneable.h"
#include "utils/case_string.h"
#include "rfc/http/header_token.h"

namespace impact {
namespace http {
    
    class chunk_extension_token {
    public:
        chunk_extension_token(std::string line);
        chunk_extension_token(std::string name, std::string value);
        ~chunk_extension_token();
        
    private:
        std::string m_name_;
        std::string m_value_;
    
    public:
        inline const std::string& name() const noexcept { return m_name_; }
        inline const std::string& value() const noexcept { return m_value_; }
    };
    
    
    class transfer_encoding;
    typedef std::shared_ptr<transfer_encoding> transfer_encoding_ptr;
    class transfer_encoding {
    public:
        typedef std::function<std::string(const std::string&)> codec_callback;
        typedef std::vector<chunk_extension_token> chunk_extension_list;
        typedef std::function<void(chunk_extension_list**)> extension_callback;
        typedef std::vector<header_token> header_list;
        typedef std::function<void(header_list**)> trailer_callback;
        
        transfer_encoding(std::string name, codec_callback&& encoder,
            codec_callback&& decoder);
        virtual ~transfer_encoding();
        
        static transfer_encoding_ptr chunked(
            extension_callback&& extension_callback = nullptr,
            trailer_callback&& trailer_callback     = nullptr
        );
        
        static void register_encoding(transfer_encoding_ptr);
        static transfer_encoding_ptr get_by_name(case_string name) noexcept;

    protected:
        codec_callback m_encode_;
        codec_callback m_decode_;
        transfer_encoding(std::string name);
        
    private:
        case_string m_name_;
        static std::map<case_string,std::shared_ptr<transfer_encoding>>
            m_encodings_;
        
        transfer_encoding();
    
    public:
        inline std::string encode(const std::string& data) const
        { return m_encode_(data); }
        inline std::string decode(const std::string& data) const
        { return m_decode_(data); }
        inline const case_string& name() const noexcept
        { return m_name_; }
        
        friend class chunked_encoding;
    };
    
    
    class chunked_encoding : public transfer_encoding {
    public:
        chunked_encoding(
            extension_callback&& extension_callback = nullptr,
            trailer_callback&& trailer_callback     = nullptr);
        ~chunked_encoding();
        
        static bool decode_first_line(
            const std::string&                  line,
            size_t*                             chunk_size,
            std::vector<chunk_extension_token>* extensions);

    private:
        extension_callback m_extension_callback_;
        trailer_callback   m_trailer_callback_;
    };
}}

#endif
