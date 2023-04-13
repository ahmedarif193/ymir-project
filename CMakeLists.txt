cmake_minimum_required(VERSION 3.10)

project(lxcd)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HTTP REQUIRED IMPORTED_TARGET libmicrohttpd)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_subdirectory(du-manager)
#add_subdirectory(liblxcd)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES "src/*.cpp")
list(APPEND SOURCES "du-manager/deployment_unit.cpp" "du-manager/deployment_unit_helper.cpp")

add_executable(lxcd ${SOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgcc_s -lc -pthread")

target_include_directories(lxcd PRIVATE lxc ${HTTP_INCLUDE_DIRS} ${LXC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include_priv)
target_link_libraries(lxcd PRIVATE PkgConfig::HTTP lxc PkgConfig::CURL json-c)

install(TARGETS lxcd DESTINATION bin)
