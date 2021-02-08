#pragma once

#include "parser.hpp"
#include "rule.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace app {
    // Фильтрации элементов DOM
    void filter(app::parser::iterator begin, app::parser::iterator end, std::vector<app::rule>& rules, std::vector<std::pair<bool, std::string>>& output, bool parent_allow = false);
    // Форматирование полученного содержимого
    void formatter(std::vector<std::pair<bool, std::string>> elements, std::ostream& output);
    // Вывод 
    void outputter(std::istream& input, const std::string& url);
    // Парсинг входных параметров
    void parse_argv(int argc, const char **argv, std::vector<std::string>& urls, std::vector<app::rule>& rules);
    // Парсинг настроек
    void parse_cfg(std::istream& input, std::vector<app::rule>& rules);
}