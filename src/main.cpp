
#include "curlpp/curlpp.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <memory>
#include <array>
#include "tidy.h"
#include "tidybuffio.h"
#include "tidyenum.h"
#include "tidyplatform.h"

std::vector<std::pair<std::string, std::regex>> irules;

// Функция получения данных от библиотеки curl
std::size_t wfunc_curl(char* src, std::size_t, std::size_t size, void* userdata);
void wfunc_tidy(TidyDoc doc, std::vector<char>& output);

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



    if (irules.empty()) {
        irules.push_back({ "tag", std::regex("(h\\d|p)") });
    }



    // Основная часть программы
    for (auto url : urls) {
        std::vector<char> d;
        std::stringstream data;
        std::cerr << url << std::endl;

        // Загрузка страницы
        try {
            curlpp::session session;

            session.set_option(curlpp::options::url(url));
            session.set_option(curlpp::options::noprogress(false));
            session.set_option(curlpp::options::wfunc(wfunc_curl));
            session.set_option(curlpp::options::wfunc_userdata(&d));
            session.run();
        }
        catch (std::exception exception) {
            std::cerr << "[error] " << exception.what() << std::endl;
            continue;
        }

        auto tdoc = tidyCreate();
        TidyBuffer tbuff;
        tidyOptSetBool(tdoc, TidyForceOutput, yes);
        tidyOptSetBool(tdoc, TidyBreakBeforeBR, no);
        tidyOptSetBool(tdoc, TidyQuoteNbsp, no);
        tidyOptSetBool(tdoc, TidyShowErrors, no);
        tidyOptSetBool(tdoc, TidyShowWarnings, no);
        tidyOptSetBool(tdoc, TidyShowInfo, no);
        tidyOptSetValue(tdoc, TidyCharEncoding, "raw");
        tidyOptSetInt(tdoc, TidyWrapLen, 0);

        tidyBufInit(&tbuff);
        tidyBufAppend(&tbuff, d.data(), d.size());
        tidyParseBuffer(tdoc, &tbuff);
        tidyCleanAndRepair(tdoc);
        d.clear();
        wfunc_tidy(tdoc, d);

        // Преобразование URL в путь
        std::string file_dir;
        std::string file_name;
        // Пути к папке, в которой будет сохранен файл
        {
            std::smatch url_parts;
            std::string url_str(url);

            std::regex_match(url_str.cbegin(), url_str.cend(), url_parts, std::regex("(([^:\\/\\?#]+):)?(\\/\\/([^\\/\\?#]*))?((?:[^\\/\\?#]*\\/)*)([^\\?#]*)?(\\?([^#]*))?(#(.*))?"));
            // std::regex_match(url_str.cbegin(), url_str.cend(), url_parts, std::regex("(([^:\\/\\?#]+):)?(\\/\\/([^\\/\\?#]*))?([^\\?#]*)(\\?([^#]*))?(#(.*))?"));

            file_dir.append(url_parts[4].first, url_parts[4].second);
            file_dir.append(url_parts[5].first, url_parts[5].second);
            file_name.append(url_parts[6].first, url_parts[6].second);
        }

        #ifdef _WIN32
        if (file_dir.front() == '/') {
            file_dir.erase(file_dir.begin());
        }
        #endif

        if (file_dir.back() == '/') {
            file_dir.erase(file_dir.end() - 1);
        }

        if (!std::filesystem::exists(file_dir) && !std::filesystem::create_directories(file_dir)) {
            std::cerr << "[error] " << "Couldn't create directory " << file_dir << std::endl;
            continue;
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
        {
            std::ofstream file(std::string(file_dir).append("/").append(file_name), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

            if (!file.is_open()) {
                std::cerr << "[error] " << "Couldn't open file " << std::string(file_dir).append("/").append(file_name) << std::endl;
                continue;
            }

            file.write(d.data(), d.size());
        }
    }
    


    return 0;
}

std::size_t wfunc_curl(char* src, std::size_t, std::size_t size, void* userdata) {
        auto buffer = reinterpret_cast<std::vector<char>*>(userdata);
    auto first = buffer->insert(buffer->end(), src, src + size);
    return std::distance(first, buffer->end());
}

void wfunc_tidy_node(TidyDoc doc, TidyNode node, std::vector<char>& output, bool write = false) {
    for (auto child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        switch (tidyNodeGetType(child)) {
        case TidyNode_Text:
            if (write) {
                TidyBuffer buffer;
                tidyBufInit(&buffer);
                tidyNodeGetText(doc, child, &buffer);
                for (auto ptr = buffer.bp; ptr < buffer.bp + buffer.size; ++ptr) {
                    if ((*(char*)ptr != '\n') && (*(char*)ptr != '\r')) {
                        output.push_back(*(char*)ptr);
                    }
                }
                tidyBufFree(&buffer);
            }

            break;
        case TidyNode_Start:
        case TidyNode_End:
        case TidyNode_StartEnd:
            auto child_name = tidyNodeGetName(child);
            auto match = false;

            std::vector<std::pair<std::string, std::string>> attrs;
            for (TidyAttr attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                attrs.push_back({ tidyAttrName(attr), tidyAttrValue(attr) ? tidyAttrValue(attr) : "" });
            }

            for (auto rule : irules) {
                if (rule.first == "tag") {
                    if (std::regex_match(child_name, rule.second)) {
                        match = true;
                    }
                }
                else {
                    for (auto attr : attrs) {
                        if ((rule.first == attr.first) && (std::regex_match(attr.second, rule.second))) {
                            match = true;
                        }
                    }
                }
            }

            wfunc_tidy_node(doc, child, output, match ? true : write);

            if (match ? true : write) {
                if (std::regex_match(child_name, std::regex("br"))) {
                    std::ostringstream endlss;
                    endlss << std::endl << std::endl;
                    std::string endls(endlss.str());
                    output.insert(output.end(), endls.begin(), endls.end());
                }
                else if (std::regex_match(child_name, std::regex("p"))) {
                    std::ostringstream endlss;
                    endlss << std::endl << std::endl;
                    std::string endls(endlss.str());
                    output.insert(output.end(), endls.begin(), endls.end());
                }
                else if (std::regex_match(child_name, std::regex("h\\d"))) {
                    std::ostringstream endlss;
                    endlss << std::endl << std::endl;
                    std::string endls(endlss.str());
                    output.insert(output.end(), endls.begin(), endls.end());
                }
                else if (std::regex_match(child_name, std::regex("a"))) {
                    for (TidyAttr attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                        if (std::regex_match(tidyAttrName(attr), std::regex("href"))) {
                            auto ptr = tidyAttrValue(attr);
                            output.push_back(' ');
                            output.push_back('[');
                            output.insert(output.end(), ptr, ptr + std::strlen(ptr));
                            output.push_back(']');
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
}

void wfunc_tidy(TidyDoc doc, std::vector<char>& output) {
    wfunc_tidy_node(doc, tidyGetRoot(doc), output);
}