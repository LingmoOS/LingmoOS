set(COMMON ${CMAKE_HOME_DIRECTORY}/src/common)

# Find systemd
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD "libsystemd" REQUIRED)
pkg_check_modules(DBUS dbus-1 REQUIRED)
pkg_check_modules(GLIB glib-2.0 REQUIRED)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SYSTEMD_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS} ${COMMON})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC appdata.cpp booster.cpp connection.cpp daemon.cpp logger.cpp
        singleinstance.cpp socketmanager.cpp
        ../common/report.c)

set(HEADERS appdata.h booster.h connection.h daemon.h logger.h launcherlib.h
    singleinstance.h socketmanager.h ${COMMON}/protocol.h)

# Set libraries to be linked. Shared libraries to be preloaded are not linked in anymore,
# but dlopen():ed and listed in src/launcher/preload.h instead.
link_libraries(${GLIB_LDFLAGS} ${DBUS_LDFLAGS} ${LIBDL} "-L/lib -lcap")

# Set executable
add_library(applauncherd SHARED ${SRC} ${MOC_SRC})
target_link_libraries(applauncherd ${SYSTEMD_LIBRARIES})

set_target_properties(applauncherd PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

# Configure and install the pkgconfig file
# ecm_generate_pkgconfig_file(
# 	BASE_NAME applauncherd
# 	FILENAME_VAR applauncherd
# 	DESCRIPTION ${PROJECT_DESCRIPTION}
# 	INSTALL)

# Add install rule
install(TARGETS applauncherd
    LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/applauncherd
    COMPONENT Devel
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
