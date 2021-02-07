#include "curl_loader.hpp"

app::curl_loader::curl_loader() : _content("") {
    return;
}

app::curl_loader::curl_loader(const std::string& url) : curl_loader() {
    load(url);
}

void app::curl_loader::load(const std::string& url) {
    curlpp::session session;
    std::ostringstream buffer;

    session.set_option(curlpp::options::url(url.c_str()));
    session.set_option(curlpp::options::noprogress(false));
    session.set_option(curlpp::options::wfunc(_wfunc));
    session.set_option(curlpp::options::wfunc_userdata(&buffer));

    session.run();

    _content = buffer.str();
}

std::string& app::curl_loader::data() {
    return _content;
}

const std::string& app::curl_loader::data() const {
    return _content;
}