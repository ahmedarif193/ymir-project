cmake_minimum_required(VERSION 3.10)

project(lxcd)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(HTTP REQUIRED IMPORTED_TARGET libmicrohttpd)

# Options
option(USE_AMXRT "Use AMX Runtime" ON)

# Add subdirectories
add_subdirectory(liblxcd)
add_subdirectory(du-manager)

# Ensure dependencies are correctly managed
add_dependencies(tr181-du-manager liblxcd)

# Source files management
set(AMX_SOURCES
    "src/backend/amx/amxhandler.cpp"
    "src/backend/amx/main.cpp"
)
set(UHTTPD_SOURCES
    "src/backend/uhttpd/httphandler.cpp"
    "src/backend/uhttpd/main.cpp"
)

# Conditionally append sources
if(USE_AMXRT)
# add_subdirectory(lxcd-supervisor)
    set(LXCD_SOURCES ${AMX_SOURCES})
else()
    set(LXCD_SOURCES ${UHTTPD_SOURCES})
endif()

# Add executable
add_executable(lxcd ${LXCD_SOURCES})
target_compile_features(lxcd PRIVATE cxx_std_17)

# Set compile options
target_compile_options(lxcd PRIVATE -fno-rtti -fno-exceptions -nostdlib++ -lgcc_s -g -lc -pthread)

# Include directories and link libraries
target_include_directories(lxcd PRIVATE
    lxc
    ${HTTP_INCLUDE_DIRS}
    ${LXC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include_priv
)
target_link_libraries(lxcd PRIVATE PkgConfig::HTTP lxc liblxcd ubox amxrt amxc amxo amxd)

# Install rules
install(TARGETS lxcd DESTINATION bin)
