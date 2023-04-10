cmake_minimum_required(VERSION 3.10)

project(tr181-du-manager)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgcc_s -lc -nodefaultlibs")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include_priv)

# Add all .cpp files in the current directory to the SOURCES list
file(GLOB SOURCES "*.cpp")

add_executable(tr181-du-manager ${SOURCES})

target_link_libraries(tr181-du-manager PRIVATE PkgConfig::JSON PkgConfig::UUID PkgConfig::CURL lxc json-c)

install(TARGETS tr181-du-manager DESTINATION bin)
