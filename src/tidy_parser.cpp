#include "tidy_parser.hpp"
#include <stdexcept>
#include <sstream>
#include <memory>

#include <iostream>

app::tidy_parser_iterator::tidy_parser_iterator(TidyDoc doc, TidyNode node) : _doc(doc), _node(node) {
    return;
}

void app::tidy_parser_iterator::next() {
    _node = tidyGetNext(_node);
}

std::unique_ptr<app::parser_iterator> app::tidy_parser_iterator::copy() const {
    return std::unique_ptr<app::tidy_parser_iterator>(new app::tidy_parser_iterator(_doc, _node));
}

app::parser_iterator_value app::tidy_parser_iterator::data() const {
    switch (tidyNodeGetType(_node)) {
    case TidyNode_Text: {
            std::ostringstream output;
            TidyBuffer buffer;
            tidyBufInit(&buffer);
            tidyNodeGetText(_doc, _node, &buffer);
            output.write(reinterpret_cast<const char*>(buffer.bp), buffer.size);
            tidyBufFree(&buffer);
            return { false, output.str() };
        }

    case TidyNode_Start:
    case TidyNode_End:
    case TidyNode_StartEnd: {
            std::ostringstream output;
            output << tidyNodeGetName(_node);
            for (TidyAttr attr = tidyAttrFirst(_node); attr; attr = tidyAttrNext(attr)) {
                output << ' ' << tidyAttrName(attr) << '=' << '\"' << (tidyAttrValue(attr) ? tidyAttrValue(attr) : "") << '\"';
            }
            return { true, output.str() };
        }
    default:
        return { false, std::string() };
    }
}

bool app::tidy_parser_iterator::equal(const app::parser_iterator& other) const {
    auto ptr = dynamic_cast<const app::tidy_parser_iterator*>(&other);
    return ptr && (_doc == ptr->_doc) && (_node == ptr->_node);
}

bool app::tidy_parser_iterator::not_equal(const app::parser_iterator& other) const {
    auto ptr = dynamic_cast<const app::tidy_parser_iterator*>(&other);
    return !ptr || (_doc != ptr->_doc) || (_node != ptr->_node);
}

std::unique_ptr<app::parser_iterator> app::tidy_parser_iterator::begin() const {
    return std::unique_ptr<app::tidy_parser_iterator>(new app::tidy_parser_iterator(_doc, tidyGetChild(_node)));
}

std::unique_ptr<app::parser_iterator> app::tidy_parser_iterator::end() const {
    return std::unique_ptr<app::tidy_parser_iterator>(new app::tidy_parser_iterator(_doc, nullptr));
}

app::tidy_parser::tidy_parser() : _doc(nullptr) {
    return;
}

app::tidy_parser::tidy_parser(const std::string& data) : tidy_parser() {
    parse(data);
}

app::tidy_parser::~tidy_parser() {
    if (_doc) {
        tidyRelease(_doc);
    }
}

void app::tidy_parser::parse(const std::string& data) {
    _doc = tidyCreate();

    tidyOptSetBool(_doc, TidyForceOutput, yes);
    tidyOptSetBool(_doc, TidyShowWarnings, no);
    tidyOptSetBool(_doc, TidyShowInfo, no);
    tidyOptSetValue(_doc, TidyCharEncoding, "raw");
    tidyOptSetInt(_doc, TidyWrapLen, 0);

    TidyBuffer buff;
    tidyBufInit(&buff);
    tidyBufAppend(&buff, const_cast<char*>(data.data()), data.size());
    
    tidyParseBuffer(_doc, &buff);
    tidyCleanAndRepair(_doc);
    tidyBufFree(&buff);
}

app::tidy_parser::iterator app::tidy_parser::root() const {
    // return std::make_unique<app::tidy_parser::iterator app::tidy_parser_iterator(_doc, tidyGetRoot(_doc)));
    return app::tidy_parser::iterator(std::make_unique<app::tidy_parser_iterator>(_doc, tidyGetRoot(_doc)));
}