# This module tries to find the Gurobi library and sets the following
# variables:
#   GUROBI_INCLUDE_DIR
#   GUROBI_LIBRARIES
#   GUROBI_FOUND

include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(GUROBI_INCLUDE_DIR
    NAMES gurobi_c++.h
    HINTS $ENV{GUROBI_HOME}
    PATH_SUFFIXES include gurobi
)
find_library(GUROBI_C_LIBRARY
    NAMES
        gurobi
        gurobi80
        gurobi90
    HINTS $ENV{GUROBI_HOME}
    PATH_SUFFIXES lib lib64
)

find_library(GUROBI_CXX_LIBRARY
    NAMES
        libgurobi_c++
        libgurobi_c++.a
        libgurobi_g++4.2
        libgurobi_g++4.2.a
        libgurobi_g++5.2
        libgurobi_g++5.2.a
    HINTS $ENV{GUROBI_HOME}
    PATH_SUFFIXES lib lib64
)

set(GUROBI_LIBRARIES "${GUROBI_CXX_LIBRARY};${GUROBI_C_LIBRARY}" )

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GUROBI DEFAULT_MSG
  GUROBI_LIBRARIES GUROBI_INCLUDE_DIR)
