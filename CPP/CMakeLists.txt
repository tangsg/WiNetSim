cmake_minimum_required(VERSION 2.8)
project(WiNetSim)

file(GLOB_RECURSE WINET_SOURCES "src/*")
file(GLOB_RECURSE WINET_HEADERS "hdr/*")

set (WINET_INCLUDE_DIRS "")
foreach (_headerFile ${WINET_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND WINET_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES WINET_INCLUDE_DIRS)

include_directories(
    ${WINET_INCLUDE_DIRS}
)

add_executable(WiNetSim ${WINET_SOURCES})
target_link_libraries(WiNetSim -pthread)
target_compile_options(WiNetSim PRIVATE -std=c++11) #(PRIVATE -fno-stack-protector)
