#include "app.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>

void app::filter(app::parser::iterator begin, app::parser::iterator end, std::vector<app::rule>& rules, std::vector<std::pair<bool, std::string>>& output, bool parent_allow) {
    for (auto current = begin; current != end; ++current) {
        auto allow = parent_allow;
        auto is_tag = current.operator*().is_tag();
        auto data = current.operator*().data();

        if (is_tag) {
            for (auto rule : rules) {
                if (rule.match(data)) {
                    allow = rule.allow();
                    break;
                }
            }
        }

        if (allow && is_tag) {
            output.push_back({ true, std::string("begin:") + data });
        }
        else if (allow) {
            output.push_back({ false, data });
        }

        app::filter(current.begin(), current.end(), rules, output, allow);

        if (allow && is_tag) {
            output.push_back({ true, std::string("end:") + data });
        }
    }
}

void app::formatter(std::vector<std::pair<bool, std::string>> elements, std::ostream& output) {
    std::size_t count = 0;
    bool nl = false;

    for (auto element : elements) {
        auto is_tag = element.first;
        auto data = element.second;
        std::smatch matches;

        // Открывающий тег
        if (is_tag && (std::regex_match(data, matches, std::regex("begin:(.*)")))) {
            auto data = matches[1].str();

            // ...
        }
        // Закрывающий тег
        else if (is_tag && (std::regex_match(data, matches, std::regex("end:(.*)")))) {
            auto data = matches[1].str();

            if (std::regex_search(data, matches, std::regex("^h\\d\\b"))) {
                if (!nl) {
                    output << std::endl << std::endl;
                    nl = true;
                }
                count = 0;
            }
            else if (std::regex_search(data, matches, std::regex("^p\\b"))) {
                if (!nl) {
                    output << std::endl << std::endl;
                    nl = true;
                }
                count = 0;
            }
            else if (std::regex_search(data, matches, std::regex("^div\\b"))) {
                if (!nl) {
                    output << std::endl << std::endl;
                    nl = true;
                }
                count = 0;
            }
            else if (std::regex_search(data, matches, std::regex("^a\\b.*href=\"([^\"]*)\".*"))) {
                if ((count + 3 + matches[1].str().size()) >= 80) {
                    output << std::endl;
                    count = 0;
                }
                output << ' ' << '[' << matches[1].str() << ']';
                count += 3 + matches[1].str().size();
                nl = false;
            }
        }
        // Текст
        else {
            std::ostringstream buffer;

            for (auto c : data) {
                if ((c != '\n') && (c != '\r')) {
                    buffer << c;
                }
            }
            auto text = buffer.str();
            auto from = text.begin();
            auto space = text.begin();

            for (auto curr = text.begin(); curr < text.end(); ++curr) {
                ++count;

                if (*curr == ' ') {
                    space = curr;
                }
                else if (((*curr & 0b11100000) == 0b11000000) && ((*(curr + 1) & 0b11000000) == 0b10000000)) {
                    curr += 1;
                }
                else if (((*curr & 0b11110000) == 0b11100000) && ((*(curr + 1) & 0b11000000) == 0b10000000) && ((*(curr + 2) & 0b11000000) == 0b10000000)) {
                    curr += 2;
                }
                else if (((*curr & 0b11111000) == 0b11110000) && ((*(curr + 1) & 0b11000000) == 0b10000000) && ((*(curr + 2) & 0b11000000) == 0b10000000) && ((*(curr + 3) & 0b11000000) == 0b10000000)) {
                    curr += 3;
                }

                if ((count >= 80) && (space != from)) {
                    output << std::string(from, space) << std::endl;
                    from = space + 1;
                    space = from;
                    count = std::distance(from, curr);
                }
            }

            if (from != text.end()) {
                output << std::string(from, text.end());
                count += std::string(from, text.end()).size();
            }
            
            nl = false;
        }
    }
}

void app::outputter(std::istream& input, const std::string& url) {
    // Преобразование URL в путь
    std::string file_dir;
    std::string file_name;

    // Пути к папке, в которой будет сохранен файл
    {
        std::smatch url_parts;
        std::regex_match(url, url_parts, std::regex("(([^:\\/\\?#]+):)?(\\/\\/([^\\/\\?#]*))?((?:[^\\/\\?#]*\\/)*)([^\\?#]*)?(\\?([^#]*))?(#(.*))?"));
        file_dir = (std::ostringstream() << url_parts[4] << url_parts[5]).str();
        file_name = (std::ostringstream() << url_parts[6]).str();
    }

    #ifdef _WIN32
    if (file_dir.front() == '/') {
        file_dir.erase(file_dir.begin());
    }
    #endif

    // Убираем / в конце пути
    if (file_dir.back() == '/') {
        file_dir.erase(file_dir.end() - 1);
    }

    // Создаем папку для выходного файла
    if (!std::filesystem::exists(file_dir) && !std::filesystem::create_directories(file_dir)) {
        throw std::runtime_error((std::ostringstream() << "Couldn't create directory: " << file_dir).str());
    }

    // Имя файла
    {
        if (file_name.empty()) {
            file_name.assign("index.txt");
        }
        else {
            std::smatch ext;
            if (std::regex_search(file_name.cbegin(), file_name.cend(), ext, std::regex("\\.[^\\.]*$"))) {
                file_name.assign(std::string(file_name.cbegin(), ext[0].first));
            }
            file_name.append(".txt");
        }
    }
        
    // Запись данных в файл
    std::string file_path = (std::ostringstream() << file_dir << '/' << file_name).str();
    {
        std::ofstream file(file_path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

        if (!file.is_open()) {
            throw std::runtime_error((std::ostringstream() << "Couldn't open for write file: " << file_path).str());
        }

        file << input.rdbuf();
    }
}

void app::parse_argv(int argc, const char **argv, std::vector<std::string>& urls, std::vector<app::rule>& rules) {
    for (int i = 1; i < argc; ++i) {
        // Настройки
        if (std::regex_match(argv[i], std::regex("-c|--config"))) {
            std::ifstream config(argv[++i]);

            if (!config.is_open()) {
                std::cerr << "[error] Couldn't open config file " << argv[i] << std::endl;
                return;
            }

            parse_cfg(config, rules);
        }
        // URL
        else if ((argv[i][0] != '-') && std::regex_match(argv[i], std::regex("(([^:\\/\\?#]+):)?(\\/\\/([^\\/\\?#]*))?([^\\?#]*)(\\?([^#]*))?(#(.*))?"))) {
            urls.push_back(argv[i]);
        }
        // Неизвестно что
        else {
            std::cerr << "[error] Unknown argument " << i << ": " << argv[i] << std::endl;
        }
    }
}

void app::parse_cfg(std::istream& input, std::vector<app::rule>& rules) {
    std::size_t lineno = 0;
    do {
        std::string line;
        std::getline(input, line);

        lineno++;
        
        // Комментарий/пустая строка
        if ((line[0] == '#') || (line.empty())) {
            continue;
        }

        // Проверка шаблона
        std::smatch line_tokens;
        if (!std::regex_match(line, line_tokens, std::regex("(allow|disallow)\\:\\s*(.*)", std::regex_constants::icase))) {
            std::cerr << "[warning] Bad rule at line " << lineno << ": " << line << std::endl;
            continue;
        }

        // Добавление правила
        if (std::regex_match(line_tokens[1].str(), std::regex("allow", std::regex_constants::icase))) {
            rules.push_back({ line_tokens[2].str(), true });
            std::cerr << "allow rule: " << line_tokens[2].str() << std::endl;
        }
        else {
            rules.push_back({ line_tokens[2].str(), false });
            std::cerr << "diallow rule: " << line_tokens[2].str() << std::endl;
        }
    }
    while (input.good());
}