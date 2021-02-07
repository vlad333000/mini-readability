#pragma once

#include "curl/curl.h"
#include "option.hpp"
#include <stdexcept>

namespace curlpp {
    class session {
    private:
        void* _handle;

    public:
        session() : _handle(curl_easy_init()) {
            if (!_handle) {
                throw std::runtime_error("can't initialize curl session");
            }
        };
        ~session() {
            curl_easy_cleanup(_handle);
        };

        template<class _Option>
        void set_option(const _Option& _Value) {
            CURLcode err = curl_easy_setopt(_handle, _Value.type(), _Value.value());
            if (err != CURLcode::CURLE_OK) {
                throw std::runtime_error(curl_easy_strerror(err));
            }
        };
        void run() {
            CURLcode err = curl_easy_perform(_handle);
            if (err != CURLcode::CURLE_OK) {
                throw std::runtime_error(curl_easy_strerror(err));
            }
        }
    };
}