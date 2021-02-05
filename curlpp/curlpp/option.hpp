#pragma once

#include "curl/curl.h"

namespace curlpp { 
    template<CURLoption _Option, class _Type>
    class option {
    private:
        _Type _value;

    public:
        option(const _Type& _Value) : _value { _Value } {
            return;
        }

        CURLoption type() const {
            return _Option;
        }
        const _Type& value() const {
            return _value;
        }
    };
}