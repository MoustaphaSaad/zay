cmake_minimum_required(VERSION 3.9)

project(zay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

add_subdirectory(mn EXCLUDE_FROM_ALL)
add_subdirectory(flag)
# add_subdirectory(rgx)
# add_subdirectory(alp)
# add_subdirectory(vm)
# add_subdirectory(zsm)
add_subdirectory(cypher)
add_subdirectory(zay)
add_subdirectory(zyc)
add_subdirectory(unittest)
