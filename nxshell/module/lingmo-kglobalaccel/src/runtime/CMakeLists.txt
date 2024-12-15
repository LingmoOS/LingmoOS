remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

###############################################################################
### KDED Global Accel Daemon

set(kglobalaccelprivate_SRCS
    kglobalacceld.cpp
    kglobalaccel_interface.cpp
    kserviceactioncomponent.cpp
    component.cpp
    logging.cpp
    globalshortcut.cpp
    globalshortcutsregistry.cpp
    globalshortcutcontext.cpp
)
ecm_create_qm_loader(kglobalaccelprivate_SRCS kglobalaccel5_qt)

configure_file(config-kglobalaccel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kglobalaccel.h )

add_library(KF5GlobalAccelPrivate ${kglobalaccelprivate_SRCS})
generate_export_header(KF5GlobalAccelPrivate BASE_NAME KF5GlobalAccelPrivate)
add_library(KF5::GlobalAccelPrivate ALIAS KF5GlobalAccelPrivate)

target_include_directories(KF5GlobalAccelPrivate INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KGlobalAccel>")

target_link_libraries(KF5GlobalAccelPrivate
   Qt5::DBus
   KF5::GlobalAccel
   KF5::WindowSystem # KKeyServer
   KF5::CoreAddons   # KAboutData
   KF5::ConfigCore
)

set_target_properties(KF5GlobalAccelPrivate PROPERTIES VERSION   ${KGLOBALACCEL_VERSION_STRING}
                                                SOVERSION ${KGLOBALACCEL_SOVERSION}
                                                EXPORT_NAME "GlobalAccelPrivate"
)

if (${XCB_XCB_FOUND})
    target_link_libraries(KF5GlobalAccelPrivate Qt5::X11Extras)
endif()

add_executable(kglobalaccel5 main.cpp logging.cpp)
ecm_mark_nongui_executable(kglobalaccel5)
target_link_libraries(kglobalaccel5
    KF5GlobalAccelPrivate
    KF5::DBusAddons   # KDBusService
    KF5::Crash
    )

add_subdirectory(plugins)

# Install application and configuration
install(TARGETS KF5GlobalAccelPrivate EXPORT KF5GlobalAccelTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install( TARGETS kglobalaccel5 ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES kglobalaccel.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME kglobalaccel5.desktop)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kf5globalaccelprivate_export.h
  kglobalacceld.h
  kglobalaccel_interface.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KGlobalAccel/private COMPONENT Devel
)

kdbusaddons_generate_dbus_service_file(kglobalaccel5 org.kde.kglobalaccel ${KDE_INSTALL_FULL_BINDIR})
