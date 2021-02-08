#pragma once
#include "parser.hpp"
#include "tidy.h"
#include "tidyenum.h"
#include "tidybuffio.h"
#include "tidyplatform.h"
#include <stack>

namespace app {
    class tidy_parser_iterator : public parser_iterator {
    public:
        tidy_parser_iterator(TidyDoc, TidyNode);

        parser_iterator_value data() const override;
        
        void next() override;
        bool equal(const parser_iterator&) const override;
        bool not_equal(const parser_iterator&) const override;
        std::unique_ptr<parser_iterator> copy() const override;
        std::unique_ptr<parser_iterator> begin() const override;
        std::unique_ptr<parser_iterator> end() const override;

    private:
        TidyDoc _doc;
        TidyNode _node;
    };

    class tidy_parser : public parser {
    public:
        using parser::iterator;

        tidy_parser();
        tidy_parser(const std::string&);
        ~tidy_parser();

        void parse(const std::string&) override;
        iterator root() const override;

    private:
        TidyDoc _doc;
    };
}