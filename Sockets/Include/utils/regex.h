/**
 * Created by TekuConcept on August 15, 2018
 */

#ifndef _IMPACT_REGEX_H_
#define _IMPACT_REGEX_H_

#include <string>
#include <vector>
#include <set>

/* TODO
    - error checking
    - range quantifiers {x[,[y]]}
    - do not count child matches when parent matches fail
      eg ((A)B) - do not count 'A' as a match when 'AB'
      is not a match
    - match chain insertion \1, \2, \x
*/

namespace impact {
namespace internal {
    class regex {
    public:
        typedef std::pair<int,int> range;
        typedef std::pair<size_t,size_t> capture;
        enum { // chars range from 0x00 - 0xFF
            START = 0x0100,
            FINISH,
            PLACEHOLDER,
            LITERAL,
            LOGIC_OR, // |
            LOGIC_01, // ?
            LOGIC_1N, // +
            GROUP_IN,
            GROUP_OUT,
            HIDDEN_GROUP_IN,
            HIDDEN_GROUP_OUT,
        };
        struct charset {
            bool not_in_set = false;
            std::string chars;
            std::vector<range> ranges;
        };
        struct atom {
            int id;
            int group_id;
            struct charset set;
            struct atom* _1;
            struct atom* _2;
            atom();
            atom(int c);
            atom(struct charset);
        };
        typedef std::shared_ptr<struct atom> atom_ptr;
    
        static std::vector<atom_ptr> generate(std::string expression);
        static std::vector<capture> match(const std::string& input,
            atom* automaton);
    
    private:
        struct chain {
            atom* root      = NULL; // current root
            atom* leaf      = NULL; // current leaf
            atom* link_root = NULL; // chain's leaf
            atom* link_leaf = NULL; // chain's root
            int id          =    0; // used for group capturing
        };
        struct parser_context {
            bool escaping      = false; // \n
            bool in_charset    = false; // [...]
            bool is_range      = false; // a '-' z
            int group_count    =    0 ; // used as identifier for groups
            int group_state    =    0 ; // (?:...) (0: default, 1: '?', 2: ':')
            int  hex_state     =    0 ; // \xHH (0: none, 1: hex, 2: MSN, 3: LSN)
            char hex_c         =  '\0';
            char last_c        =  '\0';
            atom* last_chain   =  NULL;
            atom* dangling[2]  = { NULL, NULL };
            struct charset set;         // stores temporary charset
            std::vector<atom_ptr> list; // stores all NFA nodes
            std::vector<chain> scope;   // stores all active scopes
        };
        struct capture_info {
            size_t start = 0;
            size_t end   = 0;
        };
        struct simulator_context {
            std::vector<capture> captures;
            std::vector<capture_info> history;
            std::set<atom*> current_set;
            std::set<atom*> next_set;
            int current_c   = 0;
            int current_idx = 0;
        };
        
        static char _S_char2hex(char);
        static bool _S_resolve_dangling(atom*,struct parser_context*);
        static void _S_process_next_char(char,struct parser_context*);
        static void _S_link_literal(struct parser_context*);
        static void _S_next_literal(char,struct parser_context*);
        static void _S_group_open(struct parser_context*);
        static void _S_group_close(struct parser_context*);
        static void _S_or(struct parser_context*);
        static void _S_01(struct parser_context*);
        static void _S_1N(struct parser_context*);
        static void _S_0N(struct parser_context*);
        static void _S_charset_open(struct parser_context*);
        static void _S_charset_next(char, struct parser_context*,
            bool escaped = false);
        static void _S_charset_close(struct parser_context*);
        static void _S_not(char, struct parser_context*);
        static void _S_escape(char, struct parser_context*);
        static void _S_wild_dot(struct parser_context*);
        static void _S_no_capture(char,struct parser_context*);
        
        static bool _S_charset_contains(int,const charset&);
        static bool _S_increment_states(struct simulator_context*);
        static bool _S_follow_and_compare(std::vector<atom*>*,
            struct simulator_context*);
        static void _S_check_literal(atom*,struct simulator_context*);
        static void _S_check_group_in(atom*,struct simulator_context*);
        static void _S_check_group_out(atom*,struct simulator_context*);
        
        static void print_ctx(char,struct parser_context*);
    };
}}
#endif

/* TODO
-: QUANTIFIERS :-
  {3}: Exactly three times (aaa)
{2,4}: Two to four times (aaa?a?)
 {3,}: Three or more times (aaaa*)
-: LOGIC :-
     \1: Contents of Group 1
Error Checking
*/
