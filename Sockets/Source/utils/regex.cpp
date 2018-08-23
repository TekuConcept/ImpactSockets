/**
 * Created by TekuConcept on August 15, 2018
 */

#include "utils/regex.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace impact;
using namespace internal;

#define V(x) std::cout << x << std::endl
#define ENDLINE std::cout << std::endl

/*
regex            = *atom
atom             = metacharacter / literal
metacharacter    = generic-metachar / special-char
generic-metachar = "{" / "}" / "[" / "]" / "(" / ")" /
                   "^" / "$" / "." / "|" / "*" / "+" /
                   "?" / "\"
special-char     = "\" ("d" / "s" / "w" / "D" / "S" / "W" / "N")
literal          = VCHAR / escaped-char
escaped-char     = "\" generic-metachar
*/

regex::atom::atom() : id(LITERAL), _1(NULL), _2(NULL) {}
regex::atom::atom(int c) : atom() {
    if (c <= 0x00FF) set.chars.push_back(c);
    else id = c;
}
regex::atom::atom(struct charset s) : id(LITERAL), set(s), _1(NULL), _2(NULL) {}


void
regex::print_ctx(char c,struct parser_context* __ctx)
{
    std::cout <<
        "Escaping:   " << __ctx->escaping << "\t" <<
        "In Charset: " << __ctx->in_charset << "\t" <<
        "Is Range:   " << __ctx->is_range << "\t" <<
        "Hex State:  " << __ctx->hex_state << "\t" <<
        "Hex Char:   " << __ctx->hex_c << "\t" <<
        "Crnt Char:  " << c << "\t" <<
        "Last Char:  " << __ctx->last_c << "\t" <<
        "Last Chain: " << __ctx->last_chain->id << "\t";
    std::cout << "List Size: " << __ctx->list.size() << std::endl;
    for (auto scope : __ctx->scope) {
        V("{"
        << scope.root << ", "
        << scope.leaf << ", "
        << scope.link_root << ", "
        << scope.link_leaf <<
        "}");
    }
}


std::vector<regex::atom_ptr>
regex::generate(std::string __exp)
{
    struct parser_context ctx;
    ctx.list.push_back(atom_ptr(new atom(START)));
    ctx.list.push_back(atom_ptr(new atom(FINISH)));
    atom* root = ctx.list[0].get();
    atom* link = ctx.list[1].get();
    ctx.scope.push_back(chain());
    auto current_scope       = &ctx.scope.back();
    current_scope->root      = root;
    current_scope->leaf      = root;
    current_scope->link_root = root;
    current_scope->link_leaf = link;
    ctx.last_chain           = root;
    root->group_id           = current_scope->id;
    link->group_id           = current_scope->id;
    
    for (char c : __exp) {
        print_ctx(c,&ctx);
        if (ctx.escaping) _S_escape(c,&ctx);
        else _S_process_next_char(c,&ctx);
        ctx.last_c = c;
    }
    print_ctx('\0',&ctx);
    
    current_scope = &ctx.scope.back(); // update pointer
    if (!_S_resolve_dangling(link,&ctx))
        current_scope->leaf->_1 = link;
    current_scope->leaf     = link;
    
    return ctx.list;
}


char
regex::_S_char2hex(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return (char)((c - 'A') + 10);
    if ((c >= 'a') && (c <= 'z'))
        return (char)((c - 'a') + 10);
    if ((c >= '0') && (c <= '9'))
        return (char)(c - '0');
    return 0;
}


bool
regex::_S_resolve_dangling(atom* __target, struct parser_context* __ctx)
{
    bool result = false;
    for (int i = 0; i < 2; i++) {
        if (__ctx->dangling[i]) {
            __ctx->dangling[i]->_1 = __target;
            __ctx->dangling[i] = NULL;
            result |= true;
        }
    }
    return result;
}


void
regex::_S_process_next_char(char __c, struct parser_context* __ctx)
{
    switch (__c) {
        case '[':  _S_charset_open(__ctx);     break;
        case ']':  _S_charset_close(__ctx);    break;
        case '(':  _S_group_open(__ctx);       break;
        case ')':  _S_group_close(__ctx);      break;
        case '|':  _S_or(__ctx);               break;
        case '\\': __ctx->escaping = true;     break;
        case '^':  _S_not(__c,__ctx);          break;
        case '*':  _S_0N(__ctx);               break;
        case '+':  _S_1N(__ctx);               break;
        case '?':
            if (__ctx->last_c == '(')
                __ctx->group_state++;
            else _S_01(__ctx);
            break;
        case '.':  _S_wild_dot(__ctx);         break;
        case '{':
        case '}':
        case '$':
        default:
            if (__ctx->group_state)
                 _S_no_capture(__c, __ctx);
            else _S_next_literal(__c,__ctx);
            break;
    }
}


void
regex::_S_link_literal(struct parser_context* __ctx)
{
    auto current_scope = &__ctx->scope.back();
    auto leaf          =  __ctx->list.back().get();
    
    if (!_S_resolve_dangling(leaf,__ctx))
        current_scope->leaf->_1 = leaf;
    current_scope->leaf         = leaf;
    leaf->group_id              = current_scope->id;
}


void
regex::_S_next_literal(char __c, struct parser_context* __ctx)
{
    char d = __c;
    if (__ctx->hex_state == 1) {
        __ctx->hex_c = _S_char2hex(__c);
        __ctx->hex_state++;
        return;
    }
    else if (__ctx->hex_state == 2) {
        __ctx->hex_state = 0;
        d = (char)((__ctx->hex_c << 4) |
            (0x0F & _S_char2hex(__c)));
    }
    
    if (__ctx->in_charset) _S_charset_next(__c,__ctx);
    else {
        V("default: " << d);
        __ctx->list.push_back(atom_ptr(new atom(d)));
        _S_link_literal(__ctx);
    }
}


void
regex::_S_group_open(struct parser_context* __ctx)
{
    V("Enter Group");
    
    atom_ptr root(new atom(GROUP_IN));
    atom_ptr link(new atom(GROUP_OUT));
    __ctx->scope.push_back(chain());
    __ctx->list.push_back(root);
    __ctx->list.push_back(link);
    __ctx->group_count++;
    auto parent_scope        = &__ctx->scope[__ctx->scope.size() - 2];
    auto current_scope       = &__ctx->scope.back();
    current_scope->id        = __ctx->group_count;
    current_scope->root      = root.get();
    current_scope->leaf      = root.get();
    current_scope->link_root = root.get();
    current_scope->link_leaf = link.get();
    root->group_id           = current_scope->id;
    link->group_id           = current_scope->id;
    if (!_S_resolve_dangling(current_scope->link_root,__ctx))
        parent_scope->leaf->_1 = current_scope->link_root;
    parent_scope->leaf = current_scope->link_leaf;
}


void
regex::_S_group_close(struct parser_context* __ctx)
{
    V("Exit Group");
    
    auto current_scope = __ctx->scope.back();
    __ctx->scope.pop_back();
    __ctx->last_chain = current_scope.root;
    
    if (!_S_resolve_dangling(current_scope.link_leaf,__ctx))
        current_scope.leaf->_1 = current_scope.link_leaf;
}


void
regex::_S_or(struct parser_context* __ctx)
{
    V("OR operation");
    atom_ptr op(new atom(LOGIC_OR));
    __ctx->list.push_back(op);
    
    auto current_scope      = &__ctx->scope.back();
    op->group_id            = current_scope->id;
    op->_2                  = current_scope->root->_1;
    current_scope->root->_1 = op.get();
    current_scope->root     = op.get();
    if (!_S_resolve_dangling(current_scope->link_leaf,__ctx))
        current_scope->leaf->_1 = current_scope->link_leaf;
    current_scope->leaf     = op.get();
}


void
regex::_S_01(struct parser_context* __ctx)
{
    // NOTE: _S_0N() depends on this function
    V("1 or None");
    
    atom_ptr op_ptr(new atom(LOGIC_01));
    __ctx->list.push_back(op_ptr);
    auto current_scope = &__ctx->scope.back();
    op_ptr->group_id = current_scope->id;

    atom* op = op_ptr.get();
    atom* root;
    if (__ctx->last_c == ')')
         root = __ctx->last_chain;
    else root = current_scope->leaf;
    
    std::swap(*root,*op); // swap pointer values
    std::swap( root, op); // swap local pointers
    
    op->_2             = root;
    __ctx->dangling[0] = op;
    __ctx->dangling[1] = (__ctx->last_c == ')') ?
        current_scope->leaf : root;
}


void
regex::_S_1N(struct parser_context* __ctx)
{
    V("1 or More");
    auto current_scope = &__ctx->scope.back();
    
    atom* root;
    if (__ctx->last_c == ')')
         root = __ctx->last_chain;
    else root = current_scope->leaf;
    
    atom_ptr ptr(new atom(LOGIC_1N));
    __ctx->list.push_back(ptr);

    atom* op                = ptr.get();
    current_scope->leaf->_1 = op;
    current_scope->leaf     = op;
    op->_2                  = root;
    op->group_id            = current_scope->id;
}


void
regex::_S_0N(struct parser_context* __ctx)
{
    _S_01(__ctx);
    
    atom_ptr op_1N_ptr(new atom(LOGIC_1N));
    __ctx->list.push_back(op_1N_ptr);
    
    atom* op_1N = op_1N_ptr.get();
    atom* op_01 = __ctx->dangling[0];
    atom* leaf  = __ctx->dangling[1];
    atom* root  = op_01->_2;

    op_1N->group_id    = __ctx->scope.back().id;
    op_1N->_2          = root;
    leaf->_1           = op_1N;
    __ctx->dangling[1] = op_1N;
}


void
regex::_S_charset_open(struct parser_context* __ctx)
{
    // capture charsets and ranges
    V("Enter Charset");
    __ctx->set.chars.clear();
    __ctx->set.ranges.clear();
    __ctx->in_charset = true;
}


void
regex::_S_charset_next(char c, struct parser_context* __ctx, bool escaped)
{
    if (c == '-' && !escaped) __ctx->is_range = true;
    else if (__ctx->is_range) {
        char u = __ctx->set.chars.back();
        char v = c;
        __ctx->set.chars.pop_back();
        __ctx->set.ranges.push_back(std::pair<int,int>(0x00FF&u,0x00FF&v));
        __ctx->is_range = false;
        V("range: " << u << "-" << v);
    }
    else {
        __ctx->set.chars.push_back(c);
        V("list: " << c);
    }
}


void
regex::_S_charset_close(struct parser_context* __ctx)
{
    // create node from charsets and ranges
    V("Leave Charset");
    __ctx->in_charset = false;
    __ctx->list.push_back(atom_ptr(new atom(__ctx->set)));
    _S_link_literal(__ctx);
}


void
regex::_S_not(char __c, struct parser_context* __ctx)
{
    if (__ctx->in_charset && __ctx->last_c == '[') {
        V("NOT operation");
        __ctx->set.not_in_set = true;
    }
    else _S_next_literal(__c, __ctx);
}


void
regex::_S_escape(char __c, struct parser_context* __ctx)
{
    char d;
    switch (__c) {
    case 't': d = '\t'; break;
    case 'r': d = '\r'; break;
    case 'n': d = '\n'; break;
    case 'x':
        __ctx->hex_state = 1;
        return;
    case 'R':
        _S_escape('r',__ctx);
        _S_escape('n',__ctx);
        // _S_escape('v',__ctx);
        return;
    default: d = __c; break;
    }

    if (__ctx->in_charset)
        _S_charset_next(d,__ctx,true);
    else {
        V("default: \\" << d);
        atom_ptr ptr;
        bool not_in_set = false;
        switch (d) {
        // 'x'
        case 'd': {
            ptr = atom_ptr(new atom());
            ptr->set.ranges.push_back(range('0','9'));
        } break;
        case 'w': {
            ptr = atom_ptr(new atom());
            ptr->set.chars = "_";
            ptr->set.ranges.push_back(range('a','z'));
            ptr->set.ranges.push_back(range('A','Z'));
            ptr->set.ranges.push_back(range('0','9'));
        } break;
        case 's': {
            ptr = atom_ptr(new atom());
            ptr->set.chars = " \t\n\r\v";
        } break;
        case 'h': {
            ptr = atom_ptr(new atom());
            ptr->set.chars = " \t";
        } break;
        case 'v': {
            // Line Separator:      0xE2 0x80 0xA8 (utf8, U+2028)
            // Paragraph Separator: 0xE2 0x80 0xA9 (utf8, U+2029)
            ptr = atom_ptr(new atom());
            ptr->set.chars = "\n\r\v\f";
        } break;
        case 'N':
            ptr = atom_ptr(new atom());
            ptr->set.chars = "\n\r";
        case 'H':
        case 'V':
        case 'D':
        case 'W':
        case 'S': not_in_set = true; break;
        default: ptr = atom_ptr(new atom(d)); break;
        }
        ptr->set.not_in_set = not_in_set;
        __ctx->list.push_back(ptr);
        _S_link_literal(__ctx);
    }

    __ctx->escaping = false;
}


void
regex::_S_wild_dot(struct parser_context* __ctx)
{
    if (__ctx->in_charset) _S_charset_next('.',__ctx);
    else {
        V("default: [^\\n]");
        atom_ptr ptr(new atom('\n'));
        ptr->set.not_in_set = true;
        __ctx->list.push_back(ptr);
        _S_link_literal(__ctx);
    }
}


void
regex::_S_no_capture(char __c, struct parser_context* __ctx)
{
    if (__c == ':') {
        __ctx->group_state = 0;
        __ctx->group_count--;
        // WARNING: "?:..." will result in undefined behavior
        auto parent_scope  = &__ctx->scope[__ctx->scope.size() - 2];
        auto current_scope = &__ctx->scope.back();
        current_scope->id                  = parent_scope->id;
        current_scope->link_root->group_id = parent_scope->id;
        current_scope->link_leaf->group_id = parent_scope->id;\
        current_scope->link_root->id       = HIDDEN_GROUP_IN;
        current_scope->link_leaf->id       = HIDDEN_GROUP_OUT;
    } // else error
}


std::vector<regex::capture>
regex::match(const std::string& __input, atom* __root)
{
    V("-: Simulating NFA :-");
    simulator_context ctx;
    ctx.captures = { capture(0,EOF) };
    ctx.current_set = { __root };
    std::istringstream stream(__input);
    
    do {
        ctx.current_c = stream.get();
        ctx.next_set.clear();
        
        if(_S_increment_states(&ctx)) {
            ctx.captures[0].second = ctx.current_idx - 1;
            return ctx.captures;
        } // else continue (match not found yet)
        
        // no more branches to match
        if (ctx.next_set.empty()) break;

        ctx.current_set = ctx.next_set;
        ctx.current_idx++;
    } while (ctx.current_c != EOF);
    
    ctx.captures.clear();
    return ctx.captures;
}


bool
regex::_S_charset_contains(int __c, const charset& __set)
{
    for (auto range : __set.ranges)
        if ((__c >= range.first) && (__c <= range.second))
            return true ^ __set.not_in_set;
    
    for (char e : __set.chars)
        if (__c == e)
            return true ^ __set.not_in_set;
    
    return false ^ __set.not_in_set;
}


bool
regex::_S_increment_states(struct simulator_context* __ctx)
{
    // - play through all epsilon paths until a literal is found;
    // - when a literal is found, compare it to the current char;
    // - if the char is in the literal's charset, add the next
    // state to the set;
    std::vector<atom*> stack;
    for (auto state : __ctx->current_set) {
        stack.push_back(state);
        while (stack.size())
            if (_S_follow_and_compare(&stack,__ctx))
                return true;
    }
    return false;
}


bool
regex::_S_follow_and_compare(
    std::vector<atom*>* __stack,
    struct simulator_context* __ctx)
{
    auto current_token = __stack->back();
    __stack->pop_back();
    
    switch (current_token->id) {
    case FINISH:    V("FINISHED!"); return true;
    case LITERAL:   _S_check_literal(current_token,__ctx);   break;
    case GROUP_IN:  _S_check_group_in(current_token,__ctx);  goto default_label;
    case GROUP_OUT: _S_check_group_out(current_token,__ctx);
    default:
        default_label:
        V("Branch { " << current_token->_1 <<
            ", " << current_token->_2 << " }");
        if (current_token->_1) __stack->push_back(current_token->_1);
        if (current_token->_2) __stack->push_back(current_token->_2);
        break;
    }

    return false;
}


void
regex::_S_check_literal(atom* __literal, struct simulator_context* __ctx)
{
    if (_S_charset_contains(__ctx->current_c, __literal->set)) {
        V((char)__ctx->current_c << " in state " << __literal);
        __ctx->next_set.insert(__literal->_1);
    }
    else V((char)__ctx->current_c << " not in state " << __literal);
}


void
regex::_S_check_group_in(atom* __entry, struct simulator_context* __ctx)
{
    V("Capture start at " << __ctx->current_idx <<
        " with id " << __entry->group_id);
    size_t idx = __entry->group_id;
    if (__ctx->history.size() <= idx)
        __ctx->history.resize(idx + 1);
    __ctx->history[idx].start = __ctx->current_idx;
}


void
regex::_S_check_group_out(atom* __exit, struct simulator_context* __ctx)
{
    V("Capture end at " << (__ctx->current_idx - 1) <<
        " with id " << __exit->group_id);
    auto token = &__ctx->history[__exit->group_id];
    token->end = __ctx->current_idx - 1;
    __ctx->captures.push_back(capture(token->start, token->end));
}
