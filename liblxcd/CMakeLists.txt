cmake_minimum_required(VERSION 3.10)
project(lxcd)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Define the library target and its sources
add_library(lxcd SHARED
    map.cpp
    vector.cpp
)
target_include_directories(lxcd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv)

# Set the target properties
set_target_properties(lxcd PROPERTIES
    OUTPUT_NAME "lxcd"
    VERSION 1.0.0
    SOVERSION 1
)

# Link against any necessary dependencies or libraries
#target_link_libraries(lxcd PRIVATE Threads::Threads)

# Install the library in a specified location
install(TARGETS lxcd
    LIBRARY DESTINATION lib
)