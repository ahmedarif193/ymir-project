cmake_minimum_required(VERSION 3.10)
project(lxcdutils)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Define the library target and its sources
file(GLOB SOURCES "*.cpp")
add_library(lxcdutils SHARED ${SOURCES})
target_include_directories(lxcdutils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv)

# Set the target properties
set_target_properties(lxcdutils PROPERTIES
    OUTPUT_NAME "lxcdutils"
    VERSION 1.0.0
    SOVERSION 1
)

# Link against any necessary dependencies or libraries
#target_link_libraries(lxcdutils PRIVATE Threads::Threads)

# Install the library in a specified location
install(TARGETS lxcdutils
    LIBRARY DESTINATION lib
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv/ DESTINATION include)