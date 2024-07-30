configure_file(config-kglobalaccel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kglobalaccel.h)

set(kglobalaccel_SRCS
  kglobalaccel.cpp
  kglobalshortcutinfo.cpp
  kglobalshortcutinfo_dbus.cpp
)
ecm_qt_declare_logging_category(kglobalaccel_SRCS
    HEADER kglobalaccel_debug.h
    IDENTIFIER KGLOBALACCEL_LOG
    CATEGORY_NAME kf.globalaccel
    OLD_CATEGORY_NAMES kf5.kglobalaccel
    DESCRIPTION "KGlobalAccel"
    EXPORT KGLOBALACCEL
)

ecm_create_qm_loader(kglobalaccel_SRCS kglobalaccel6_qt)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_xml} kglobalaccel_interface)
install(FILES ${kglobalaccel_xml} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf6_org.kde.KGlobalAccel.xml)

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface)
install(FILES ${kglobalaccel_component_xml} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf6_org.kde.kglobalaccel.Component.xml)

add_library(KF6GlobalAccel ${kglobalaccel_SRCS})
add_library(KF6::GlobalAccel ALIAS KF6GlobalAccel)

ecm_generate_export_header(KF6GlobalAccel
    BASE_NAME KGlobalAccel
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.102
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6GlobalAccel INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KGlobalAccel>")

target_link_libraries(KF6GlobalAccel
PUBLIC
    Qt6::DBus
    Qt6::Widgets
)

if(HAVE_X11)
    target_link_libraries(KF6GlobalAccel PRIVATE Qt6::GuiPrivate) # qtx11extras_p.h
endif()

set_target_properties(KF6GlobalAccel PROPERTIES VERSION   ${KGLOBALACCEL_VERSION}
                                                SOVERSION ${KGLOBALACCEL_SOVERSION}
                                                EXPORT_NAME "GlobalAccel"
)

ecm_generate_headers(KGlobalAccel_HEADERS
  HEADER_NAMES
  KGlobalAccel
  KGlobalShortcutInfo

  REQUIRED_HEADERS KGlobalAccel_HEADERS
)

install(TARGETS KF6GlobalAccel EXPORT KF6GlobalAccelTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kglobalaccel_export.h
  ${KGlobalAccel_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KGlobalAccel COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6GlobalAccel_QCH
        NAME KGlobalAccel
        BASE_NAME KF6GlobalAccel
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KGlobalAccel_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Widgets_QCH
            Qt6DBus_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KGLOBALACCEL_EXPORT
            KGLOBALACCEL_DEPRECATED
            KGLOBALACCEL_DEPRECATED_EXPORT
            "KGLOBALACCEL_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT KGLOBALACCEL
    FILE kglobalaccel.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
