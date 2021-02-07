cmake_minimum_required(VERSION 3.0)

project(mini_readability)

add_subdirectory(curl)
add_subdirectory(curlpp)
add_subdirectory(tidy-html5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(mini_readability ${CMAKE_SOURCE_DIR}/src/main.cpp)

target_link_libraries(mini_readability libcurlpp)
target_include_directories(mini_readability PRIVATE ${CMAKE_SOURCE_DIR}/tidy-html5/include)

if(${BUILD_SHARED_LIB})
    target_link_libraries(mini_readability tidy-share)
else()
    target_link_libraries(mini_readability tidy-static)
endif()