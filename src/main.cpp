
#include "curlpp/curlpp.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <memory>

// Функция получения данных от библиотеки curl
std::size_t wfunc(char* src, std::size_t, std::size_t size, void* userdata);

int main(int argc, const char **argv)
{
    // Парсинг входных параметров
    std::vector<const char*> urls;
    
    for (int i = 1; i < argc; ++i) {
        auto arg = argv[i];

        if (arg[0] == '-') {
            if ((strcmp(arg, "-c") == 0) || (strcmp(arg, "--config") == 0)) {
                auto config = argv[++i];
            }
            else {
                std::cerr << "unknown argument: " << arg << std::endl;
            }
        }
        else {
            urls.push_back(arg);
        }
    }



    // Основная часть программы
    for (auto url : urls) {
        std::vector<char> data;

        // Загрузка страницы
        try {
            curlpp::session session;

            session.set_option(curlpp::options::url(url));
            session.set_option(curlpp::options::noprogress(true));
            session.set_option(curlpp::options::wfunc(wfunc));
            session.set_option(curlpp::options::wfunc_userdata(&data));
            session.run();
        }
        catch (std::exception exception) {
            std::cerr << "[error] " << url << ": " << exception.what() << std::endl;
        }

        // Преобразование URL в путь
        std::stringstream path;

        std::match_results<const char*> matches;
        std::regex_match(url, matches, std::regex("(([^:\\/\\?#]+):)?(\\/\\/([^\\/\\?#]*))?([^\\?#]*)(\\?([^#]*))?(#(.*))?"));
        path << matches[4] << matches[5];

        std::filesystem::create_directories(path.str());

        path << '/' << "index.html";
        
        // Запись данных в файл
        std::ofstream file(path.str(), std::ios_base::out | std::ios_base::trunc);
        file.write(data.data(), data.size());
        file.close();
    }
    


    return 0;
}

std::size_t wfunc(char* src, std::size_t, std::size_t size, void* userdata) {
    auto buffer = reinterpret_cast<std::vector<char>*>(userdata);
    
    buffer->resize(buffer->size() + size);
    std::memcpy(buffer->data() + buffer->size() - size, src, size);

    return size;
}