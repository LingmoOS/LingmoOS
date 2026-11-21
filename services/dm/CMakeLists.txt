cmake_minimum_required(VERSION 3.22)

project(lingmo-dm
    VERSION 1.0
    DESCRIPTION "Lingmo display manager"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
option(BUILD_TESTING "Build the test suite" OFF)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_library(PAM_LIBRARY pam)
if(NOT PAM_LIBRARY)
    message(FATAL_ERROR "lingmo-dm needs libpam")
endif()

set(DM_HELPER_PATH ${CMAKE_INSTALL_FULL_LIBEXECDIR}/lingmo-dm-helper)
set(DM_SESSION_WRAPPER ${CMAKE_INSTALL_FULL_LIBEXECDIR}/lingmo-dm-session)

add_library(lingmo-dm-common STATIC common/wire.cpp)
target_link_libraries(lingmo-dm-common PUBLIC Lingmo::SDKBase)

add_executable(lingmo-dm
    daemon/backgrounds.cpp
    daemon/config.cpp
    daemon/daemon.cpp
    daemon/greeterserver.cpp
    daemon/helperprocess.cpp
    daemon/main.cpp
    daemon/sessions.cpp
    daemon/state.cpp
    daemon/terminal.cpp
)
target_compile_definitions(lingmo-dm PRIVATE
    DM_HELPER_PATH="${DM_HELPER_PATH}"
    DM_SESSION_WRAPPER="${DM_SESSION_WRAPPER}"
)
target_link_libraries(lingmo-dm PRIVATE lingmo-dm-common)

# No event loop and no threads: PAM modules and the session's fork run here.
add_executable(lingmo-dm-helper helper/main.cpp)
target_link_libraries(lingmo-dm-helper PRIVATE lingmo-dm-common ${PAM_LIBRARY})

install(TARGETS lingmo-dm DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS lingmo-dm-helper DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS data/lingmo-dm-session DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES data/lingmo-dm.service DESTINATION /usr/lib/systemd/system)
install(FILES data/lingmo-dm.pam DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/pam.d
        RENAME lingmo-dm)
install(FILES data/lingmo-dm-greeter.pam DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/pam.d
        RENAME lingmo-dm-greeter)
install(FILES data/lingmo-dm-autologin.pam DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/pam.d
        RENAME lingmo-dm-autologin)
install(FILES data/org.lingmo.DisplayManager.conf
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system.d)
foreach(unit pipewire.socket pipewire.service pipewire-pulse.socket pipewire-pulse.service
        wireplumber.service filter-chain.service)
    install(FILES data/no-greeter-audio.conf
            DESTINATION /usr/lib/systemd/user/${unit}.d
            RENAME lingmo-dm.conf)
endforeach()

lingmo_add_translations(lingmo-dm SOURCES daemon helper)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
