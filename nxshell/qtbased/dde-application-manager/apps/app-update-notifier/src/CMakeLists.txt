add_subdirectory(dbus)

add_executable(${APP_UPDATE_NOTIFIER_BIN} main.cpp)

target_link_libraries(${APP_UPDATE_NOTIFIER_BIN} PRIVATE
    app_update_notifier_static
)

target_include_directories(${APP_UPDATE_NOTIFIER_BIN} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

include(GNUInstallDirs)
install(TARGETS ${APP_UPDATE_NOTIFIER_BIN} DESTINATION ${AM_LIBEXEC_DIR})
