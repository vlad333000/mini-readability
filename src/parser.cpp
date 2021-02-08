#include "parser.hpp"

app::parser_iterator_value::parser_iterator_value(bool is_tag, const std::string& data) : _is_tag(is_tag), _data(data) {
    return;
}

bool app::parser_iterator_value::is_tag() const {
    return _is_tag;
}

std::string app::parser_iterator_value::data() const {
    return _data;
}

app::_parser_iterator::_parser_iterator(std::unique_ptr<parser_iterator> it) : _it(nullptr) {
    _it.swap(it);
}

app::_parser_iterator::_parser_iterator(const app::_parser_iterator& other) : _it(nullptr) {
    if (other._it) {
        _it = other._it->copy();
    }
}

app::_parser_iterator app::_parser_iterator::operator++() {
    _it->next();
    return *this;
}

app::_parser_iterator app::_parser_iterator::operator++(int) {
    auto tmp = app::_parser_iterator(*this);
    operator++();
    return *this;
}

app::_parser_iterator::value_type app::_parser_iterator::operator*() const {
    return _it->data();
}

bool app::_parser_iterator::operator==(const app::_parser_iterator& other) const {
    return _it->equal(*other._it);
}

bool app::_parser_iterator::operator!=(const app::_parser_iterator& other) const {
    return _it->not_equal(*other._it);
}

app::_parser_iterator app::_parser_iterator::begin() const {
    return app::_parser_iterator(_it->begin());
}

app::_parser_iterator app::_parser_iterator::end() const {
    return app::_parser_iterator(_it->end());
}