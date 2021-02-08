#include "rule.hpp"

app::rule::rule(const std::string& regex, bool allow) : _regex(regex), _allow(allow) {
    return;
}

bool app::rule::match(const std::string& text) const {
    return std::regex_match(text, _regex);
}

bool app::rule::allow() const {
    return _allow;
}