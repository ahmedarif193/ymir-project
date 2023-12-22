cmake_minimum_required(VERSION 3.10)
project(liblxcd)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Define the library target and its sources
file(GLOB SOURCES "*.cpp")
add_library(liblxcd SHARED ${SOURCES})
target_include_directories(liblxcd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv)

pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

# Set the target properties
set_target_properties(liblxcd PROPERTIES
    OUTPUT_NAME "lxcd"
    VERSION 1.0.0
    SOVERSION 1
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-threadsafe-statics -fno-rtti -fno-exceptions -nostdlib++ -pthread")
target_link_libraries(liblxcd PRIVATE lxc json-c curl PkgConfig::CURL)

# Install the library in a specified location
install(TARGETS liblxcd
    LIBRARY DESTINATION lib
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv/ DESTINATION include)