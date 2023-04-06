cmake_minimum_required(VERSION 3.10)

project(lxcd)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LXC REQUIRED IMPORTED_TARGET lxc)
pkg_check_modules(HTTP REQUIRED IMPORTED_TARGET libmicrohttpd)
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET jsoncpp)
add_subdirectory(du-manager)

file(GLOB SOURCES "src/*.cpp")

add_executable(lxcd ${SOURCES})

target_include_directories(lxcd PRIVATE ${JSON_INCLUDE_DIRS} ${HTTP_INCLUDE_DIRS} ${LXC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include_priv)
target_link_libraries(lxcd PRIVATE PkgConfig::LXC PkgConfig::HTTP PkgConfig::JSON)

install(TARGETS lxcd DESTINATION bin)
