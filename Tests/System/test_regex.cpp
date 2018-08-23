/**
 * Created by TekuConcept on August 23, 2018
 */

#include "utils/regex.h"

#include <iostream>

using namespace impact;
using namespace internal;

std::string id2str(int id) {
    switch(id) {
    case regex::START:            return "START";
    case regex::FINISH:           return "FINISH";
    case regex::PLACEHOLDER:      return "PLACEHOLDER";
    case regex::LITERAL:          return "LITERAL";
    case regex::LOGIC_OR:         return "LOGIC |";
    case regex::LOGIC_01:         return "LOGIC ?";
    case regex::LOGIC_1N:         return "LOGIC +";
    case regex::GROUP_IN:         return "BEGIN";
    case regex::GROUP_OUT:        return "END";
    case regex::HIDDEN_GROUP_IN:  return "HIDDEN I";
    case regex::HIDDEN_GROUP_OUT: return "HIDDEN O";
    default:                      return "(unknown)";
    }
}

void print_list(struct regex::atom* root) {
    std::vector<struct regex::atom*> stack;
    stack.push_back(root);
    while (stack.size()) {
        auto n = stack.back();
        stack.pop_back();
        if (n->_1)
            stack.push_back(n->_1);
        if (n->_2 && n->id != regex::LOGIC_1N)
            stack.push_back(n->_2);
        std::cout << "state: ";
        if (n->id == regex::LITERAL) {
            std::cout << "[" << n->set.chars;
            for (const auto& p : n->set.ranges)
                std::cout << (char)p.first << '-' << (char)p.second;
            std::cout << "]";
        }
        else std::cout << id2str(n->id);
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "- BEGIN -" << std::endl;
    
    // std::string pattern = "[A-Z]ello World";
    // std::string pattern = "f(oo)*";
    // std::string pattern = "foo*";
    // std::string pattern = "Fo(o)+";
    // std::string pattern = "foo+";
    // std::string pattern = "(?:Say )?(Hello|Goodbye)";
    // std::string pattern = "(a)?(b)";
    // std::string pattern = "ab?((c?d)e)";
    // std::string pattern = "a?(b)";
    // std::string pattern = "(a)?b";
    // std::string pattern = "(a)?";
    // std::string pattern = "(ab?)c";
    // std::string pattern = "ab?|c";
    // std::string pattern = "a|bc?"; // a|(bc?)
    // std::string pattern = "a?b";
    // std::string pattern = "(Hello|Goodbye|My) World!";
    // std::string pattern = "((Hello) World)";
    // std::string pattern = "Hello World";
    
    // std::string pattern = "(today)|(tomorrow)";
    std::string pattern = "(?:(HTTP/)([0-9])\\.([0-9]) |([A-Za-z0-9#-'^-`!\\*\\+\\-.\\|~]+ ))";
    auto automaton = regex::generate(pattern);
    print_list(automaton[0].get());
    
    for (auto state : automaton) {
        std::cout <<
            "ID:    " << id2str(state->id) << "\t" <<
            "Group: " << state->group_id << "\t" <<
            "Self:  " << state.get() << "\t" <<
            "Left:  " << state->_1 << "\t" <<
            "Right: " << state->_2 << std::endl;
        std::cout << "Chars: " << state->set.chars << std::endl;
        for (auto r : state->set.ranges) {
            std::cout << "Range: " << r.first << "-" << r.second << std::endl;
        }
    }
    
    // std::string input = "HTTP/1.1 200 OK";
    std::string input = "GET / HTTP/1.1";
    auto result = regex::match(input, automaton[0].get());
    
    std::cout << "\n\"" << input << "\"" <<
    (result.size() > 0 ? " matches " : " does not match ") <<
    "\"" << pattern << "\"" << std::endl;
    
    std::cout << "\nMatches: " << std::endl;
    for (auto capture : result) {
        std::cout << "- " << input.substr(
            capture.first,
            (capture.second + 1) - capture.first) << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "- END OF LINE -" << std::endl;
    return 0;
}