cmake_minimum_required(VERSION 3.5)
project(Prism)

# Normal flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -Wextra -Wpedantic -Wshadow -O3 -ffast-math")

# Our source files.
file(GLOB SOURCES "*.cpp")

# Our binary.
add_executable(prism ${SOURCES})

# What to link with.
target_link_libraries(prism m pthread)

# We need these C++ features.
target_compile_features(prism PRIVATE cxx_thread_local)

# If we're on MacOS, add minifb.
if(APPLE)
    message("-- Adding minifb to display rendered images")

    # Add the library directory.
    add_subdirectory(minifb minifb)

    # Tell our code to use it.
    target_compile_definitions(prism PRIVATE DISPLAY)
    target_include_directories(prism PRIVATE ${minifb_SOURCE_DIR})

    # Need the Cocoa framework.
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(prism minifb ${COCOA_LIBRARY})
endif()
