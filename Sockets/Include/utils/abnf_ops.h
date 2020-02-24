/**
 * Created by TekuConcept on August 1, 2018
 */

#ifndef IMPACT_ABNF_OPS_H
#define IMPACT_ABNF_OPS_H

namespace impact {
namespace internal {
    inline bool ALPHA(char c) { return ((c >= '\x41') && (c <= '\x5A')) || ((c >= '\x61') && (c <= '\x7A')); }
    inline bool BIT(char c) { return c == '0' || c == '1'; }
    inline bool CHAR(char c) { return (c >= '\x01') && ((c == '\x7F') || (c < '\x7F')); }
    inline bool LF(char c) { return c == '\x0A'; }
    inline bool CR(char c) { return c == '\x0D'; }
    inline bool CRLF(char a, char b) { return CR(a) && LF(b); }
    inline bool CTL(char c) { return (c == '\x7F') || (c == '\x00') || ((c > '\x00') && (c <= '\x1F')); }
    inline bool DIGIT(char c) { return (c >= '\x30') && (c <= '\x39'); }
    inline bool DQUOTE(char c) { return c == '\x22'; }
    inline bool HEXDIG(char c) { return DIGIT(c) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')); }
    inline bool HTAB(char c) { return c == '\x09'; }
    inline bool OCTET(char c) { return ((c > '\x00') && (c < '\xFF')) || (c == '\x00') || (c == '\xFF'); }
    inline bool SP(char c) { return c == '\x20'; }
    inline bool VCHAR(char c) { return (c >= '\x21') && (c <= '\x7E'); }
    inline bool WSP(char c) { return SP(c) || HTAB(c); }
}}
