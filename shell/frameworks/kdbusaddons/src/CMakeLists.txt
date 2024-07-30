add_subdirectory(tools/kquitapp)

add_library(KF6DBusAddons)
add_library(KF6::DBusAddons ALIAS KF6DBusAddons)

set_target_properties(KF6DBusAddons PROPERTIES
    VERSION     ${KDBUSADDONS_VERSION}
    SOVERSION   ${KDBUSADDONS_SOVERSION}
    EXPORT_NAME DBusAddons
)

ecm_create_qm_loader(KF6DBusAddons kdbusaddons6_qt)

target_sources(KF6DBusAddons PRIVATE
    kdbusservice.cpp
    kdbusservice.h
    kdedmodule.cpp
    kdedmodule.h
    kupdatelaunchenvironmentjob.cpp
    kupdatelaunchenvironmentjob.h
)

ecm_qt_declare_logging_category(KF6DBusAddons
    HEADER kdbusaddons_debug.h
    IDENTIFIER KDBUSADDONS_LOG
    CATEGORY_NAME kf.dbusaddons
    OLD_CATEGORY_NAMES kf5.kdbusaddons
    DESCRIPTION "KDBusAddons"
    EXPORT KDBUSADDONS
)

set(libkdbusaddons_dbus_SRCS)
qt_add_dbus_interface(libkdbusaddons_dbus_SRCS org.freedesktop.Application.xml FreeDesktopApplpicationIface)
qt_add_dbus_interface(libkdbusaddons_dbus_SRCS org.kde.KDBusService.xml KDBusServiceIface)

qt_add_dbus_adaptor(libkdbusaddons_dbus_SRCS
   org.freedesktop.Application.xml
   kdbusservice.h
   KDBusService
   kdbusservice_adaptor
   KDBusServiceAdaptor)
qt_add_dbus_adaptor(libkdbusaddons_dbus_SRCS
   org.kde.KDBusService.xml
   kdbusservice.h
   KDBusService
   kdbusserviceextensions_adaptor
   KDBusServiceExtensionsAdaptor)

target_sources(KF6DBusAddons PRIVATE
    ${libkdbusaddons_dbus_SRCS}
)

ecm_generate_export_header(KF6DBusAddons
    BASE_NAME KDBusAddons
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6DBusAddons PUBLIC Qt6::DBus)

if(HAVE_X11)
    target_link_libraries(KF6DBusAddons PRIVATE Qt6::GuiPrivate) # qtx11extras_p.h
endif()

target_include_directories(KF6DBusAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KDBusAddons>")

configure_file(config-kdbusaddons.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdbusaddons.h )

ecm_generate_headers(KDBusAddons_HEADERS
  HEADER_NAMES
  KDBusService
  KDEDModule
  KUpdateLaunchEnvironmentJob
  REQUIRED_HEADERS KDBusAddons_HEADERS
)

install(TARGETS KF6DBusAddons EXPORT KF6DBusAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KDBusAddons_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDBusAddons COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KDBUSADDONS
    FILE kdbusaddons.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6DBusAddons_QCH
        NAME KDBusAddons
        BASE_NAME KF6DBusAddons
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KDBusAddons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6DBus_QCH
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KDBUSADDONS_EXPORT
            KDBUSADDONS_DEPRECATED
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

