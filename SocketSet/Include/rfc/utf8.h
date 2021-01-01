/**
 * Created by TekuConcept on July 28, 2018
 */

#ifndef IMPACT_RFC_UTF8_H
#define IMPACT_RFC_UTF8_H

#include <cstdint>
#include <string>
#include <vector>

#define RFC3629 1

namespace impact {
    class utf8 {
    public:
        utf8() = delete; /* just a namespace */

        static bool serialize(char32_t input, std::string* result);
        static bool serialize(const std::string& input, std::string* result);
        static bool serialize(const std::u16string& input, std::string* result);
        static bool serialize(const std::u32string& input, std::string* result);
        static bool deserialize(const std::string& input, std::u32string* result);

    private:
        static inline size_t _S_estimate_buf_size(char32_t __symbol) {
            if (__symbol <= 0x00007F) return 1;
            if (__symbol <= 0x0007FF) return 2;
            if (__symbol <= 0x00FFFF) {
                if (__symbol >= 0x00D800 && __symbol <= 0x00DFFF)
                    return static_cast<size_t>(-1);
                return 3;
            }
            if (__symbol <= 0x10FFFF) return 4;
            return static_cast<size_t>(-1);
        }
        static inline void _S_encode(
            const char32_t& __symbol,
            std::string*    __result) {
            /*
                RFC3629 Section 3. UTF-8 definition
                Char. number range  |        UTF-8 octet sequence
                   (hexadecimal)    |              (binary)
                --------------------+-------------------------------------------
                0000 0000-0000 007F | 0xxxxxxx
                0000 0080-0000 07FF | 110xxxxx 10xxxxxx
                0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
                0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            */
            if (__symbol <= 0x00007F) {
                __result->push_back(static_cast<char>(__symbol));
                return;
            }
            if (__symbol <= 0x0007FF) {
                __result->push_back(static_cast<char>(0xC0|(__symbol>>6)));
                goto tail_a;
            }
            if (__symbol <= 0x00FFFF) {
                __result->push_back(static_cast<char>(0xE0|(__symbol>>12)));
                goto tail_b;
            }
            __result->push_back(static_cast<char>(0xF0|(__symbol>>18)));
            __result->push_back(0x80|((__symbol>>12)&0x3F));
            tail_b: __result->push_back(0x80|((__symbol>>6)&0x3F));
            tail_a: __result->push_back(0x80|(__symbol&0x3F));
        }

        friend class test_utf8_c; /* guts private access */
    };
}

#endif
