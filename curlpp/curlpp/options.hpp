#pragma once

#include "option.hpp"

#include <cstddef>

namespace curlpp {
    using wfunc_t = typename std::size_t (*)(char *, std::size_t, std::size_t, void*);

    namespace options {
        using url = option<CURLoption::CURLOPT_URL, const char*>;
        using noprogress = option<CURLoption::CURLOPT_NOPROGRESS, bool>;
        using wfunc = option<CURLoption::CURLOPT_WRITEFUNCTION, wfunc_t>;
        using wfunc_userdata = option<CURLoption::CURLOPT_WRITEDATA, void*>;
        using charset = option<CURLoption::CURLOPT_ENCODING, const char*>;
    }
}