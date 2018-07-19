cmake_minimum_required(VERSION 3.0)

project("FIR Solver LP - New version (Gurobi)"
    LANGUAGES CXX
)

option(LP_DEBUG "Activate debug build" OFF)

add_definitions(-Wall -Wextra -g -O2 -std=c++11)

if (LP_DEBUG)
    message(STATUS "Debug: enable")
    add_definitions(-DDEBUG)
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/module"
)

find_package(Gurobi REQUIRED)
include_directories(${GUROBI_INCLUDE_DIR})

add_executable(fir-solver
    src/main.cc
    src/local/Fir.cc
    src/local/LinearProgram.cc
    src/local/ScriptGenerator.cc
    src/local/TclProject.cc
)

target_link_libraries(fir-solver
    ${GUROBI_LIBRARIES}
)
