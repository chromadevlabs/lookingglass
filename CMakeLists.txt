cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 20)
project(lookingglass CXX OBJCXX)

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/macos_app.mm
)

target_include_directories(lookingglass
    PRIVATE
        thirdparty/json/single_include
)

target_compile_options(lookingglass
    PRIVATE
        "-Werror"
)

target_link_libraries(lookingglass
    PRIVATE
        "-framework Cocoa"
        "-framework Webkit"
)
