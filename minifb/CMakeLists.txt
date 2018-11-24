cmake_minimum_required (VERSION 3.5)
project (minifb)

# Add source files.
file(GLOB SOURCES "*.m")

# Our library.
add_library(minifb STATIC ${SOURCES})
