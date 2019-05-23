cmake_minimum_required (VERSION 3.10)
project(dbgext)

# Configure the debug extension with the minimal sources needed
add_library(dbgext SHARED "dbgext.cpp" "dbgext.h" "dbgext.rc")

# Add the implementation specific sources
target_sources(dbgext PRIVATE "src/extension.cpp" "src/extension.h" "src/object.cpp" "src/object.h")
target_sources(dbgext PRIVATE "src/v8.cpp" "src/v8.h" "src/v8-layout.h" "src/v8-layout.cpp")

# Should be found in the Windows SDK if using the right Developer Tools Prompt
# Note: Finding them before linking them results in better errors if not found
find_library(DBGENG_LIB DbgEng)
find_library(DBGMODEL_LIB DbgModel)
find_library(RUNTIMEOBJECT_LIB RuntimeObject)

# DbgEng and DbgModel are needed for Debugger extensions. RuntimeObject for COM.
target_link_libraries(dbgext ${DBGENG_LIB})
target_link_libraries(dbgext ${DBGMODEL_LIB})
target_link_libraries(dbgext ${RUNTIMEOBJECT_LIB})

# C++17 is required for some of the header files
set_property(TARGET dbgext PROPERTY CXX_STANDARD 17)
set_property(TARGET dbgext PROPERTY CXX_STANDARD_REQUIRED ON)
