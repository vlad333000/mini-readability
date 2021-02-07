#pragma once

#include <string>

namespace app {

class loader {
public:
    virtual void load(const std::string&) = 0;
    virtual std::string& data() = 0;
    virtual const std::string& data() const = 0;
};

}