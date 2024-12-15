find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

file(GLOB APP_LAUNCH_HELPER_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_executable(${APP_LAUNCH_HELPER_BIN} ${APP_LAUNCH_HELPER_SOURCE})

target_link_libraries(${APP_LAUNCH_HELPER_BIN} PRIVATE
    PkgConfig::SYSTEMD
)

target_include_directories(${APP_LAUNCH_HELPER_BIN} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

include(GNUInstallDirs)
install(TARGETS ${APP_LAUNCH_HELPER_BIN} DESTINATION ${AM_LIBEXEC_DIR})
