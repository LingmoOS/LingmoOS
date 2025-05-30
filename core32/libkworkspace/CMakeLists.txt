
set(kworkspace_LIB_SRCS kdisplaymanager.cpp
    kworkspace.cpp
    sessionmanagement.cpp
    sessionmanagementbackend.cpp
    updatelaunchenvjob.cpp
    autostartscriptdesktopfile.cpp
    kdisplaymanager.h
    kworkspace.h
    sessionmanagement.h
    sessionmanagementbackend.h
    updatelaunchenvjob.h
    autostartscriptdesktopfile.h
    login1_manager_interface.cpp
    login1_manager_interface.h
   )

add_definitions(-DTRANSLATION_DOMAIN=\"libkworkspace\")

qt_add_dbus_interface(kworkspace_LIB_SRCS kf5_org.kde.KLauncher.xml klauncher_interface)
qt_add_dbus_interface(kworkspace_LIB_SRCS ${KSCREENLOCKER_DBUS_INTERFACES_DIR}/kf5_org.freedesktop.ScreenSaver.xml screenlocker_interface )
qt_add_dbus_interface(kworkspace_LIB_SRCS ${KSCREENLOCKER_DBUS_INTERFACES_DIR}/org.kde.screensaver.xml kscreenlocker_interface )
qt_add_dbus_interface(kworkspace_LIB_SRCS ${lingmo-workspace_SOURCE_DIR}/ksmserver/org.kde.LogoutPrompt.xml logoutprompt_interface)
qt_add_dbus_interface(kworkspace_LIB_SRCS ${lingmo-workspace_SOURCE_DIR}/startshell/lingmo-session/org.kde.Startup.xml startup_interface)
qt_add_dbus_interface(kworkspace_LIB_SRCS ${lingmo-workspace_SOURCE_DIR}/startshell/lingmo-poweroff/org.kde.Shutdown.xml shutdown_interface)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Seat.xml"
                            "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Session.xml"
    PROPERTIES INCLUDE "loginddbustypes.h" )

qt_add_dbus_interface(kworkspace_LIB_SRCS "org.freedesktop.UPower.xml" upower_interface)
qt_add_dbus_interface(kworkspace_LIB_SRCS "org.freedesktop.ConsoleKit.Manager.xml" consolekit_manager_interface)

set(ksmserver_xml ${lingmo-workspace_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt_add_dbus_interface( kworkspace_LIB_SRCS ${ksmserver_xml} ksmserver_interface )

add_library(kworkspace ${kworkspace_LIB_SRCS})
add_library(PW::KWorkspace ALIAS kworkspace)
set_target_properties(kworkspace PROPERTIES
                             VERSION ${PROJECT_VERSION}
                             SOVERSION ${PROJECT_VERSION_MAJOR}
                             EXPORT_NAME KWorkspace
                             OUTPUT_NAME kworkspace5
                      )

generate_export_header(kworkspace)
target_link_libraries(kworkspace
    PUBLIC
        Qt::Core
        KF5::CoreAddons
    PRIVATE
        Qt::DBus
        KF5::I18n
        KF5::WindowSystem
        KF5::ConfigCore
)
target_include_directories(kworkspace PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                      INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kworkspace5>" )

configure_file(config-libkworkspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libkworkspace.h)

if(HAVE_X11)
    target_link_libraries(kworkspace PRIVATE X11::Xau)
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(kworkspace PRIVATE Qt::X11Extras)
    else()
        target_link_libraries(kworkspace PRIVATE Qt::GuiPrivate)
    endif()
endif()

write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/LibKWorkspaceConfigVersion.cmake VERSION
                                     "${PROJECT_VERSION}" COMPATIBILITY AnyNewerVersion)


ecm_qt_declare_logging_category(kworkspace
    HEADER libkworkspace_debug.h
    IDENTIFIER LIBKWORKSPACE_DEBUG
    CATEGORY_NAME org.kde.plasma.libkworkspace
)

install(TARGETS kworkspace EXPORT libkworkspaceLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kworkspace.h
               sessionmanagement.h
               updatelaunchenvjob.h
               autostartscriptdesktopfile.h
               ${CMAKE_CURRENT_BINARY_DIR}/config-libkworkspace.h
               ${CMAKE_CURRENT_BINARY_DIR}/kworkspace_export.h
         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kworkspace5 COMPONENT Devel )

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_CMAKEPACKAGEDIR}/LibKWorkspace)
configure_package_config_file(LibKWorkspaceConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/LibKWorkspaceConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/LibKWorkspaceConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/LibKWorkspaceConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(EXPORT libkworkspaceLibraryTargets
        NAMESPACE PW::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE LibKWorkspaceLibraryTargets.cmake )

if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()
