set(kglobalaccelprivate_SRCS
    kglobalacceld.cpp
    kglobalaccel_interface.cpp
    kserviceactioncomponent.cpp
    component.cpp
    logging.cpp
    globalshortcut.cpp
    globalshortcutsregistry.cpp
    globalshortcutcontext.cpp
    sequencehelpers_p.cpp
)

configure_file(config-kglobalaccel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kglobalaccel.h )

add_library(KGlobalAccelD ${kglobalaccelprivate_SRCS})
add_library(K::KGlobalAccelD ALIAS KGlobalAccelD)

ecm_generate_export_header(KGlobalAccelD
    EXPORT_FILE_NAME kglobalacceld_export.h
    BASE_NAME KGlobalAccelD
    VERSION ${PROJECT_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KGlobalAccel
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.3 5.90
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KGlobalAccelD INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KGlobalAccelD>")

target_link_libraries(KGlobalAccelD
    PUBLIC
        KF6::GlobalAccel
    PRIVATE
        Qt6::DBus
        KF6::WindowSystem # KKeyServer
        KF6::CoreAddons   # KAboutData
        KF6::ConfigCore
        KF6::Service
        KF6::KIOGui
        KF6::JobWidgets
)

set_target_properties(KGlobalAccelD PROPERTIES VERSION ${KGLOBALACCELD_VERSION}
                                                SOVERSION ${KGLOBALACCELD_SOVERSION}
                                                EXPORT_NAME "KGlobalAccelD"
)

target_compile_definitions(KGlobalAccelD PRIVATE -DQDBUS=${qdbus_EXECUTABLE})

if(XCB_XCB_FOUND)
    target_link_libraries(KGlobalAccelD PUBLIC Qt6::GuiPrivate) # qtx11extras_p.h
endif()

add_executable(kglobalacceld main.cpp logging.cpp)

target_include_directories(kglobalacceld PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(kglobalacceld
    KGlobalAccelD
    KF6::CoreAddons
    KF6::DBusAddons   # KDBusService
    KF6::Crash
    )

add_subdirectory(plugins)

install(TARGETS KGlobalAccelD EXPORT KGlobalAccelDTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install(TARGETS kglobalacceld DESTINATION ${KDE_INSTALL_LIBEXECDIR})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kglobalacceld_export.h
  kglobalacceld.h
  kglobalaccel_interface.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KGlobalAccelD/ COMPONENT Devel
)

include (ECMConfiguredInstall)
ecm_install_configured_files(
   INPUT plasma-kglobalaccel.service.in
   DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)

ecm_install_configured_files(
    INPUT kglobalacceld.desktop.in
    DESTINATION ${KDE_INSTALL_AUTOSTARTDIR} @ONLY
)
