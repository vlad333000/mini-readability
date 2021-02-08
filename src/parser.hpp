#pragma once

#include <iterator>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>

namespace app {
    class parser_iterator_value {
    public:
        parser_iterator_value(bool, const std::string&);

        bool is_tag() const;
        std::string data() const;

    private:
        bool _is_tag;
        std::string _data;
    };

    class parser_iterator {
    public:
        virtual parser_iterator_value data() const = 0;
        
        virtual bool equal(const parser_iterator&) const = 0;
        virtual bool not_equal(const parser_iterator&) const = 0;
        virtual void next() = 0;
        virtual std::unique_ptr<parser_iterator> copy() const = 0;
        virtual std::unique_ptr<parser_iterator> begin() const = 0;
        virtual std::unique_ptr<parser_iterator> end() const = 0;
    };

    class _parser_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = parser_iterator_value;

        _parser_iterator(std::unique_ptr<parser_iterator>);
        _parser_iterator(const _parser_iterator&);

        _parser_iterator operator++();
        _parser_iterator operator++(int);
        value_type operator*() const;

        bool operator==(const _parser_iterator&) const;
        bool operator!=(const _parser_iterator&) const;

        _parser_iterator begin() const;
        _parser_iterator end() const;
    private:
        std::unique_ptr<parser_iterator> _it;
    };

    class parser {
    public:
        using iterator = _parser_iterator;
        
        virtual void parse(const std::string&) = 0;
        virtual iterator root() const = 0;
    };
}