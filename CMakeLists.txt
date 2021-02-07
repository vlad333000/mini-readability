cmake_minimum_required(VERSION 3.0)

project(mini_readability)

option(APP_USE_CURL "Use libcurl for loader" ON)
option(APP_USE_TIDY "Use libtidy for parsing HTML" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mini_readability ${CMAKE_SOURCE_DIR}/src/main.cpp)

if(${APP_USE_CURL})
    add_subdirectory(curl)
    target_link_libraries(mini_readability PRIVATE libcurl)
    target_compile_definitions(mini_readability PRIVATE APP_USE_CURL)
    target_sources(mini_readability PRIVATE ${CMAKE_SOURCE_DIR}/src/curl_loader.cpp)
endif()

if(${APP_USE_TIDY})
    add_subdirectory(tidy-html5)
    target_include_directories(mini_readability PRIVATE ${CMAKE_SOURCE_DIR}/tidy-html5/include)
    if(${BUILD_SHARED_LIB})
        target_link_libraries(mini_readability PRIVATE tidy-share)
    else()
        target_link_libraries(mini_readability PRIVATE tidy-static)
    endif()
    target_compile_definitions(mini_readability PRIVATE APP_USE_TIDY)
    # target_sources(mini_readability PRIVATE ${CMAKE_SOURCE_DIR}/src/tidy_parser.cpp)
endif()