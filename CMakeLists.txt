cmake_minimum_required (VERSION 3.8)
project(dbgext)

# The DLL to build
add_library(dbgext SHARED "dbgext.cpp" "dbgext.h" "lib/DbgModelClientEx.h")

# Should be found in the Windows SDK if using the right Developer Tools Prompt
find_library(DBGENG_LIB DbgEng)
find_library(DBGMODEL_LIB DbgModel)

target_link_libraries(dbgext ${DBGENG_LIB})
target_link_libraries(dbgext ${DBGMODEL_LIB})

# C++17 is required for some of the header files
set_property(TARGET dbgext PROPERTY CXX_STANDARD 17)
set_property(TARGET dbgext PROPERTY CXX_STANDARD_REQUIRED ON)
