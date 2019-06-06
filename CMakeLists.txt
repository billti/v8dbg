cmake_minimum_required (VERSION 3.10)
project(dbgext)

# C++17 is required for some of the header files
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the preprocessor definitions for Unicode
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)

# Configure the debug extension with the minimal sources needed
add_library(dbgext SHARED "dbgext.cpp" "dbgext.h" "dbgext.rc" "utilities.cpp" "utilities.h")

# Add the implementation specific sources
target_sources(dbgext PRIVATE "src/extension.cpp" "src/extension.h" "src/object.cpp" "src/object.h")
target_sources(dbgext PRIVATE "src/v8.cpp" "src/v8.h" "src/v8-layout.h" "src/curisolate.cpp" "src/curisolate.h")

# Add the test binary
add_executable(testcons "test/main.cpp" "test/common.h")

# DbgEng and DbgModel are needed for Debugger extensions. RuntimeObject for COM.
target_link_libraries(dbgext DbgEng DbgModel RuntimeObject)
target_link_libraries(testcons DbgEng DbgModel RuntimeObject)
