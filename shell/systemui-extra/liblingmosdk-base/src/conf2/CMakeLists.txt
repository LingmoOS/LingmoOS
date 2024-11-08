
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GTK_INCLUDE_DIRS})

include_directories(api)

add_subdirectory(api)
add_subdirectory(sync-config)

add_executable(kconf2 tools/kconf2-tool.c )
target_link_libraries(kconf2 kyconf2)

# add_executable(kconf2-editor tools/kconf2-editor.c )
# target_link_libraries(kconf2-editor kyconf2 ${GTK_LIBRARIES})

add_executable(kyconf2-test test/kyconf2-test.c)
target_link_libraries(kyconf2-test kyconf2)

install(TARGETS kconf2
        DESTINATION /usr/bin)

# install(TARGETS kconf2-editor
#         DESTINATION /usr/bin)

install(TARGETS kyconf2-test
        DESTINATION /usr/bin)
