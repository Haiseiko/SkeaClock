cmake_minimum_required(VERSION 3.10.2)
project(native-lib C CXX)

find_library(log-lib log)
find_library(vulkan-lib vulkan)

add_library(native-lib SHARED
    native-lib.cpp
    vulkan_renderer.cpp
    texture_loader.cpp
)

target_include_directories(native-lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(native-lib
    ${vulkan-lib}
    ${log-lib}
)

set_target_properties(native-lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# If you add stb_image implementation (stb_image.h / STB_IMAGE_IMPLEMENTATION),
# add it to the sources above or a separate compilation unit and define HAS_STB_IMAGE
# for the native target, e.g.:
# target_compile_definitions(native-lib PRIVATE HAS_STB_IMAGE=1)
