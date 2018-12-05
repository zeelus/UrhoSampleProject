# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Set project name
project (SampleProject)

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
include (UrhoCommon)

set (TARGET_NAME SampleProjectTarget)

file (GLOB CPP_FILES *.cpp)
file (GLOB_RECURSE HPP_FILES *.hpp)
set (SOURCE_FILES ${CPP_FILES} ${HPP_FILES})

setup_main_executable ()

add_compile_options ("-std=c++11")
