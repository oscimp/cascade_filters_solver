cmake_minimum_required(VERSION 3.0)

project("FIR Solver LP - New version (Gurobi)"
    LANGUAGES CXX
    VERSION 1.0
)

option(LP_DEBUG "Activate debug build" OFF)

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CXXFLAGS
# assume it's a GNU-like compiler (GCC, clang)
set(CMAKE_CXX_FLAGS                 "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG           "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE         "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")

if (LP_DEBUG)
  message(STATUS "Debug: enable")
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/module"
)

find_package(Gurobi REQUIRED)
include_directories(${GUROBI_INCLUDE_DIR})

add_executable(fir-solver
  src/main.cc
  src/local/Fir.cc
  src/local/MaximizeRejection.cc
  src/local/MinimizeArea.cc
  src/local/QuadraticProgram.cc
  src/local/ScriptGenerator.cc
  src/local/TclADC.cc
  src/local/TclPRN.cc
  src/local/TclProject.cc
)

target_link_libraries(fir-solver
  ${GUROBI_LIBRARIES}
)
