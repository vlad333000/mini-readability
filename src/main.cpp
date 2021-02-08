#include "app.hpp"
#include "rule.hpp"

#include "loader.hpp"
#ifdef APP_USE_CURL
    #include "curl_loader.hpp"
#endif

#include "parser.hpp"
#ifdef APP_USE_TIDY
    #include "tidy_parser.hpp"
#endif



int main(int argc, const char **argv)
{
    // Парсинг входных параметров
    std::vector<std::string> urls;
    std::vector<app::rule> rules;
    app::parse_argv(argc, argv, urls, rules);

    // Стандартные правила - все h и p теги
    if (rules.empty()) {
        rules.push_back({ "(h\\d|p).*" });
    }

    // Основная часть программы
    for (auto url : urls) {
        std::cerr << url << std::endl;

        // Загрузка страницы
        #ifdef APP_USE_CURL
        app::loader& url_content = app::curl_loader(url);
        #else
        #error "No lib for loader defined"
        #endif

        // Парсинг HTML
        #ifdef APP_USE_TIDY
        app::parser& url_dom = app::tidy_parser(url_content.data());
        #else
        #error "No lib for parser defined"
        #endif

        // Обход DOM и фильтрация содержимого
        std::vector<std::pair<bool, std::string>> elements;
        app::filter(url_dom.root().begin(), url_dom.root().end(), rules, elements);
        
        // Форматирование
        std::ostringstream buffer;
        app::formatter(elements, buffer);

        // Вывод
        app::outputter(std::istringstream(buffer.str()), url);
    }
    
    return 0;
}