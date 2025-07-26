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

#set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE TRUE
#)
#
#set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
#)
#
#set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE_INFO_PLIST_CONTENT
#    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
#<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
#<plist version=\"1.0\">
#<dict>
#    <key>NSHighResolutionCapable</key>
#    <true/> <!-- Recommended for modern macOS apps -->
#    <key>LSUIElement</key>
#    <false/> <!-- Set to false to show in Dock/Task Bar -->
#</dict>
#</plist>"
#)
