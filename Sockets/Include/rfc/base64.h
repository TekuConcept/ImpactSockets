/**
 * Created by TekuConcept on July 25, 2017
 */

#ifndef _IMPACT_RFC_BASE64_H_
#define _IMPACT_RFC_BASE64_H_

#include <string>

namespace impact {
    class base64 {
    public:
        static std::string encode(const std::string& data);
		static std::string decode(const std::string& data)
			/* throw(std::runtime_error) */;
        static std::string decode(const std::string& data, bool& status);

    private:
        static const std::string k_alphabet;
        static const char        k_pad;

        base64();
        static unsigned char reverse_lookup(const char c);
    };
}

#endif
