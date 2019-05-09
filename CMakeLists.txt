# CMakeList.txt : CMake project for dbgext, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(dbgext)

# Add source to this project's executable.
add_library(dbgext SHARED "dbgext.cpp" "dbgext.h")

find_library(DBGENG_LIB DbgEng)
find_library(DBGMODEL_LIB DbgModel)

target_link_libraries(dbgext ${DBGENG_LIB})
target_link_libraries(dbgext ${DBGMODEL_LIB})
