set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Set sources
set(SRC invokelib.cpp invoker.cpp ${COMMON}/report.c search.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${DBUS_INCLUDE_DIRS} ${COMMON})

# Set precompiler flags
add_definitions(-DPROG_NAME_INVOKER="deepin-turbo-invoker")

# Set target
add_executable(deepin-turbo-invoker ${SRC})

target_link_libraries(deepin-turbo-invoker ${DBUS_LDFLAGS})

# Add install rule
install(TARGETS deepin-turbo-invoker DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)

set(PROG_FILE ${CMAKE_INSTALL_PREFIX}/bin/deepin-turbo-invoker)
configure_file(binfmt.conf.in binfmt.conf)
install_files(
    "/lib/binfmt.d"
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/binfmt.conf
)
