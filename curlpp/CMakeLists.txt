project(curlpp)

add_library(libcurlpp INTERFACE)
target_include_directories(libcurlpp INTERFACE ./)
target_link_libraries(libcurlpp INTERFACE libcurl)