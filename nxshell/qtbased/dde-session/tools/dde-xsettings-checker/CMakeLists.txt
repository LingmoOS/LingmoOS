set(BIN_NAME dde-xsettings-checker)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GIO2 REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(XCURSOR REQUIRED IMPORTED_TARGET xcursor)
pkg_check_modules(XFIXES REQUIRED IMPORTED_TARGET xfixes)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

add_executable(${BIN_NAME}
    main.cpp
    xsettingschecker.h
    xsettingschecker.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-session/impl/iowait/xcursor_remap.h
    ${PROJECT_SOURCE_DIR}/src/dde-session/impl/iowait/xcursor_remap.c
    ${PROJECT_SOURCE_DIR}/src/utils/utils.h
    )

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    PkgConfig::QGSettings
    PkgConfig::GLIB2
    PkgConfig::GIO2
    PkgConfig::QGSettings
    PkgConfig::XCURSOR
    PkgConfig::XFIXES
    PkgConfig::X11
    )

target_include_directories(${BIN_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src/dde-session/impl/iowait
    ${PROJECT_SOURCE_DIR}/src/utils
    )

install(TARGETS ${BIN_NAME} DESTINATION bin)

set(SYSTEMD_FILES
    dde-xsettings-checker.service
)
install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
install_symlink(dde-xsettings-checker.service dde-session-initialized.target.wants)
