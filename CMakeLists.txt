cmake_minimum_required(VERSION 3.10)

project(lxcd)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LXC REQUIRED IMPORTED_TARGET lxc)
pkg_check_modules(HTTP REQUIRED IMPORTED_TARGET libmicrohttpd)

file(GLOB SOURCES "src/*.cpp")

add_executable(lxcd ${SOURCES})

target_include_directories(lxcd PRIVATE ${HTTP_INCLUDE_DIRS} ${LXC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include_priv)
target_link_libraries(lxcd PRIVATE PkgConfig::LXC PkgConfig::HTTP)

install(TARGETS lxcd DESTINATION bin)
