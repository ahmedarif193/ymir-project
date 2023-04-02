cmake_minimum_required(VERSION 3.10)

project(tr181-du-manager)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)

pkg_check_modules(LXC REQUIRED IMPORTED_TARGET lxc)
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET jsoncpp)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
file(GLOB SOURCES "*.cpp")

add_executable(tr181-du-manager ${SOURCES})

target_include_directories(tr181-du-manager PRIVATE ${JSON_INCLUDE_DIRS} ${LXC_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include_priv)
target_link_libraries(tr181-du-manager PRIVATE PkgConfig::LXC PkgConfig::JSON PkgConfig::UUID)

install(TARGETS tr181-du-manager DESTINATION bin)
