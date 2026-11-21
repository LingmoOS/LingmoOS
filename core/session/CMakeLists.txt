project(lingmo-session)
set(TARGET lingmo-session)

pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(SOURCES
    application.cpp
    autostartentries.cpp
    journalstream.cpp
    main.cpp
    processmanager.cpp
    restartpolicy.cpp
    compositorprocess.cpp
    xwaylandenvironment.cpp
    shellprocess.cpp
    shellconnection.cpp
    power.cpp
    userdirectories.cpp
)

add_executable(${TARGET} ${SOURCES})
# The sources name their Qt objects with QString; qt_interop.h converts at the
# toolkit's std::string boundary. Force-included so it is in scope everywhere.
# One token: CMake drops a repeated '-include' option but keeps its argument,
# which would then be compiled as a second source file.
target_compile_options(${TARGET} PRIVATE
    -include${CMAKE_SOURCE_DIR}/qt_interop.h
)
target_link_libraries(${TARGET}
    lingmo-core-common
    Lingmo::SDKBase
    Qt6::Core
    PkgConfig::Systemd
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS lingmo-wayland-session DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES document-portal-stop.conf
    DESTINATION /usr/lib/systemd/user/xdg-document-portal.service.d
    RENAME lingmo-session.conf)
