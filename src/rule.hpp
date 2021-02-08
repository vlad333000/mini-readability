#pragma once

#include <regex>
#include <string>

namespace app {
    class rule {
    public:
        rule(const std::string&, bool = true);

        bool match(const std::string&) const;
        bool allow() const;

    private:
        std::regex _regex;
        bool _allow;
    };
}