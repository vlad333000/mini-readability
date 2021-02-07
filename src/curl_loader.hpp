#pragma once
#include "loader.hpp"
#include "curlpp/curlpp.hpp"
#include <sstream>

namespace app {
    class curl_loader : public loader {
    public:
        curl_loader();
        curl_loader(const std::string& url);

        void load(const std::string&);
        std::string& data() override;
        const std::string& data() const override;

    private:
        std::string _content;
        
        static std::size_t _wfunc(char* src, std::size_t, std::size_t size, void* userdata) {
            reinterpret_cast<std::ostream*>(userdata)->write(src, size);
            return reinterpret_cast<std::ostream*>(userdata)->good() ? size : 0;
        }
    };
}