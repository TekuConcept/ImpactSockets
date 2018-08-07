/**
 * Created by TekuConcept on July 29, 2018
 */

#include "generator.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#define DMESG(x) std::cout << x << std::endl

inline bool ALPHA(char c) { return ((c >= '\x41') && (c <= '\x5A')) || ((c >= '\x61') && (c <= '\x7A')); }
inline bool BIT(char c) { return c == '0' || c == '1'; }
inline bool CHAR(char c) { return (c >= '\x01') && (c <= '\x7F'); } // *
inline bool LF(char c) { return c == '\x0A'; }
inline bool CR(char c) { return c == '\x0D'; }
inline bool CRLF(char a, char b) { return CR(a) && LF(b); }
inline bool CTL(char c) { return (c == '\x7F') || ((c >= '\x00') && (c <= '\x1F')); }
inline bool DIGIT(char c) { return (c >= '\x30') && (c <= '\x39'); }
inline bool DQUOTE(char c) { return c == '\x22'; }
inline bool HEXDIG(char c) { return DIGIT(c) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')); }
inline bool HTAB(char c) { return c == '\x09'; }
// inline bool lwsp() { /* *(WSP / CRLF WSP) */ }
inline bool OCTET(char c) { return (c >= '\x00') && (c <= '\xFF'); }
inline bool SP(char c) { return c == '\x20'; }
inline bool VCHAR(char c) { return (c >= '\x21') && (c <= '\x7E'); }
inline bool WSP(char c) { return SP(c) || HTAB(c); }



inline bool comment(std::istringstream* __stream) {
    // comment        =  ";" *(WSP / VCHAR) CRLF
    auto offset = __stream->tellg();
    auto success = false;
    
    char c = __stream->get();
    success = (c == ';');
    if (success) {
        char a = __stream->get();
        while (WSP(a) || VCHAR(a))
            a = __stream->get();
        char b = __stream->get();
        success = CRLF(a,b);
    }
    
    if (!success) {
        __stream->clear();
        __stream->seekg(offset,std::ios::beg);
    }
    return success;
}


inline bool c_nl(std::istringstream* __stream) {
    // c-nl           =  comment / CRLF
    //                   ; comment or newline
    auto offset = __stream->tellg();
    auto success = comment(__stream) ||
        CRLF((char)__stream->get(), (char)__stream->get());
    if (!success) {
        __stream->clear();
        __stream->seekg(offset,std::ios::beg);
    }
    return success;
}


inline bool c_wsp(std::istringstream* __stream) {
    // c-wsp          =  WSP / (c-nl WSP)
    auto offset = __stream->tellg();
    auto success = WSP((char)__stream->peek()) ||
        (c_nl(__stream) && WSP(__stream->get()));
    if (!success) {
        __stream->clear();
        __stream->seekg(offset,std::ios::beg);
    }
    return success;
}


inline bool rulename(std::istringstream* __stream) {
    // rulename       =  ALPHA *(ALPHA / DIGIT / "-")
    if (ALPHA(__stream->peek())) {
        char c;
        do {
            __stream->get();
            c = __stream->peek();
        } while (ALPHA(c) || DIGIT(c) || (c == '-'));
        return true;
    }
    return false;
}


inline bool defined_as(std::istringstream* __stream) {
    // defined-as     =  *c-wsp ("=" / "=/") *c-wsp
    //                   ; basic rules definition and
    //                   ;  incremental alternatives
    auto offset = __stream->tellg();
    bool success = true;
    
    while (c_wsp(__stream)) continue;
    
    char a = __stream->get();
    char b = __stream->peek();
    if (a == '=' && b == '/') {
        __stream->get();
        success = true;
    }
    else success = (a == '=');
    
    if (success)
        while (c_wsp(__stream)) continue;
    
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


inline bool char_val(std::istringstream* __stream) {
    // char-val       =  DQUOTE *(%x20-21 / %x23-7E) DQUOTE
    //                        ; quoted string of SP and VCHAR
    //                        ;  without DQUOTE
    auto offset = __stream->tellg();
    bool success = false;
    
    char c = __stream->get();
    if (!DQUOTE(c)) goto char_val_done;
    
    c = __stream->peek();
    while (((c >= '\x20') && (c <= '\x21')) || ((c >= '\x23') && (c <= '\x7E'))) {
        __stream->get();
        c = __stream->peek();
    }
    
    c = __stream->get();
    success = DQUOTE(c);
    
char_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


inline bool bin_val(std::istringstream* __stream) {
    // bin-val        =  "b" 1*BIT
    //                   [ 1*("." 1*BIT) / ("-" 1*BIT) ]
    //                        ; series of concatenated bit values
    //                        ;  or single ONEOF range
    auto offset = __stream->tellg();
    bool success = false;
    bool next;
    
    const int k_type_none  = 0;
    const int k_type_range = 1;
    const int k_type_space = 2;
    int extension_type = k_type_none;
    
    char c = __stream->get();
    if (c != 'd') goto bin_val_done;
    
    do {
        next = false;
        c = __stream->get();
        if (!BIT(c)) {
            success = false;
            goto bin_val_done;
        }
        while (BIT(__stream->peek())) __stream->get();
        success = true;
        
        c = __stream->peek();
        if (c == '-') {
            if (extension_type == k_type_none)
                extension_type = k_type_range;
            else {
                success = false;
                goto bin_val_done;
            }
            __stream->get();
            next = true;
        }
        else if (c == '.') {
            if (extension_type == k_type_none)
                extension_type = k_type_space;
            else if (extension_type != k_type_space) {
                success = false;
                goto bin_val_done;
            }
            __stream->get();
            next = true;
        }
    } while(next);
    
bin_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


inline bool dec_val(std::istringstream* __stream) {
    // dec-val        =  "d" 1*DIGIT
    //                   [ 1*("." 1*DIGIT) / ("-" 1*DIGIT) ]
    auto offset = __stream->tellg();
    bool success = false;
    bool next;
    
    const int k_type_none  = 0;
    const int k_type_range = 1;
    const int k_type_space = 2;
    int extension_type = k_type_none;
    
    char c = __stream->get();
    if (c != 'd') goto dec_val_done;
    
    do {
        next = false;
        c = __stream->get();
        if (!DIGIT(c)) {
            success = false;
            goto dec_val_done;
        }
        while (DIGIT(__stream->peek())) __stream->get();
        success = true;
        
        c = __stream->peek();
        if (c == '-') {
            if (extension_type == k_type_none)
                extension_type = k_type_range;
            else {
                success = false;
                goto dec_val_done;
            }
            __stream->get();
            next = true;
        }
        else if (c == '.') {
            if (extension_type == k_type_none)
                extension_type = k_type_space;
            else if (extension_type != k_type_space) {
                success = false;
                goto dec_val_done;
            }
            __stream->get();
            next = true;
        }
    } while(next);
    
dec_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


inline bool hex_val(std::istringstream* __stream) {
    // hex-val        =  "x" 1*HEXDIG
    //                   [ 1*("." 1*HEXDIG) / ("-" 1*HEXDIG) ]
    auto offset  = __stream->tellg();
    bool success = false;
    bool next;
    
    const int k_type_none  = 0;
    const int k_type_range = 1;
    const int k_type_space = 2;
    int extension_type = k_type_none;
    
    char c = __stream->get();
    if (c != 'x') goto hex_val_done;
    
    do {
        next = false;
        c = __stream->get();
        if (!HEXDIG(c)) {
            success = false;
            goto hex_val_done;
        }
        while (HEXDIG(__stream->peek())) __stream->get();
        success = true;
        
        c = __stream->peek();
        if (c == '-') {
            if (extension_type == k_type_none)
                extension_type = k_type_range;
            else {
                success = false;
                goto hex_val_done;
            }
            __stream->get();
            next = true;
        }
        else if (c == '.') {
            if (extension_type == k_type_none)
                extension_type = k_type_space;
            else if (extension_type != k_type_space) {
                success = false;
                goto hex_val_done;
            }
            __stream->get();
            next = true;
        }
    } while(next);
    
hex_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


inline bool num_val(std::istringstream* __stream) {
    // num-val        =  "%" (bin-val / dec-val / hex-val)
    auto offset = __stream->tellg();
    auto success = false;
    
    if (__stream->get() != '%') goto num_val_done;
    success = bin_val(__stream) || dec_val(__stream) || hex_val(__stream);

num_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    return success;
}


inline bool prose_val(std::istringstream* __stream) {
    // prose-val      =  "<" *(%x20-3D / %x3F-7E) ">"
    //                        ; bracketed string of SP and VCHAR
    //                        ;  without angles
    //                        ; prose description, to be used as
    //                        ;  last resort
    auto offset = __stream->tellg();
    bool success = false;
    
    char c = __stream->get();
    if (!(c == '<')) goto prose_val_done;
    
    c = __stream->peek();
    while (((c >= '\x20') && (c <= '\x3D')) || ((c >= '\x3F') && (c <= '\x7E'))) {
        __stream->get();
        c = __stream->peek();
    }
    
    c = __stream->get();
    success = (c == '>');
    
prose_val_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return success;
}


bool option(std::istringstream*);
bool group(std::istringstream*);
inline bool element(std::istringstream* __stream) {
    // element        =  rulename / group / option /
    //                   char-val / num-val / prose-val
    return rulename(__stream) || group(__stream) || option(__stream) ||
        char_val(__stream) || num_val(__stream) || prose_val(__stream);
}


inline bool repeat(std::istringstream* __stream) {
    // repeat         =  1*DIGIT / (*DIGIT "*" *DIGIT)
    auto offset = __stream->tellg();
    bool success = false;
    bool check_next = false;
    
    char c = __stream->get();
    
    if (c == '*') check_next = true;
    else if (DIGIT(c)) {
        while (DIGIT(__stream->peek())) __stream->get();
        if (__stream->peek() == '*') {
            __stream->get();
            check_next = true;
        }
    }
    else goto repeat_done;
    success = true;

    if(check_next)
        while (DIGIT(__stream->peek())) __stream->get();

repeat_done:    
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    return success;
}


inline bool repetition(std::istringstream* __stream) {
    // repetition     =  [repeat] element
    repeat(__stream);
    return element(__stream);
}


inline bool concatenation(std::istringstream* __stream) {
    // concatenation  =  repetition *(1*c-wsp repetition)
    auto offset = __stream->tellg();
    bool success = false;
    bool a, b, n = false;
    
    success = repetition(__stream);
    if (success) offset = __stream->tellg();
    
    while (true) {
        a = c_wsp(__stream);
        if (a) {
            while (c_wsp(__stream)) continue;
            b = repetition(__stream);
        }
        if (a && b) offset = __stream->tellg();
        else {
            n = a || b;
            break;
        }
    }
    
    if (!success || n) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    return success;
}


inline bool alternation(std::istringstream* __stream) {
    // alternation    =  concatenation
    //                   *(*c-wsp "/" *c-wsp concatenation)
    auto offset = __stream->tellg();
    bool success = false;
    bool n = false;
    
    success = concatenation(__stream);
    if (success) offset = __stream->tellg();
    
    while (true) {
        while (c_wsp(__stream)) continue;
        if (__stream->get() != '/') {
            n = true;
            break;
        }
        while (c_wsp(__stream)) continue;
        if (!concatenation(__stream)) {
            n = true;
            break;
        }
        offset = __stream->tellg();
    }
    
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    return success;
}


bool group(std::istringstream* __stream) {
    // group          =  "(" *c-wsp alternation *c-wsp ")"
    auto offset = __stream->tellg();
    bool success = false;
    
    char c = __stream->get();
    if (c != '(') goto group_done;
    while (c_wsp(__stream)) continue;
    if (!alternation(__stream)) goto group_done;
    while (c_wsp(__stream)) continue;
    c = __stream->get();
    success = (c == ')');

group_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return false;
}


bool option(std::istringstream* __stream) {
    // option         =  "[" *c-wsp alternation *c-wsp "]"
    auto offset = __stream->tellg();
    bool success = false;
    
    char c = __stream->get();
    if (c != '[') goto option_done;
    while (c_wsp(__stream)) continue;
    if (!alternation(__stream)) goto option_done;
    while (c_wsp(__stream)) continue;
    c = __stream->get();
    success = (c == ']');

option_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    
    return false;
}


inline bool elements(std::istringstream* __stream) {
    // elements       =  alternation *c-wsp
    if (alternation(__stream)) {
        while (c_wsp(__stream)) continue;
        return true;
    }
    return false;
}


inline bool rule(std::istringstream* __stream) {
    // rule           =  rulename defined-as elements c-nl
    //                   ; continues if next line starts
    //                   ;  with white space
    auto offset = __stream->tellg();
    bool success, next = false;
    
    if (!(success = rulename(__stream))) goto rule_done;
    if (!(success = defined_as(__stream))) goto rule_done;
    do {
        if (next) while (c_wsp(__stream)) continue;
        if (!(success = elements(__stream))) goto rule_done;
        if (!(success = c_nl(__stream))) goto rule_done;
        next = true;
    } while (c_wsp(__stream));

rule_done:
    if (!success) {
        __stream->clear();
        __stream->seekg(offset, std::ios::beg);
    }
    return success;
}


inline bool rulelist(std::string __grammar) {
    // rulelist       =  1*( rule / (*c-wsp c-nl) )
    bool success = false;
    std::transform(
        __grammar.begin(),
        __grammar.end(),
        __grammar.begin(),
        ::tolower
    );
    std::istringstream stream(__grammar);
    do {
        if (rule(&stream)) {
            success = true;
            continue;
        }
        else {
            while (c_wsp(&stream)) {
                success = true;
                continue;
            }
            if (c_nl(&stream)) {
                success = true;
                continue;
            }
            break;
        }
    } while(true);
    return success;
}


int main() {
    DMESG("- BEGIN -");

    //std::string test = "zip-code         = 5DIGIT [\"-\" 4DIGIT]";
    std::stringstream test;
    test << "; test comment \r\n";
    
    auto status = rulelist(test.str());
    DMESG("Success: " << (status?"true":"false"));
    
    DMESG("- END OF LINE -");
    return 0;
}
