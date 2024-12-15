configure_file(config-kglobalaccel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kglobalaccel.h )

set(kglobalaccel_SRCS
  kglobalaccel.cpp
  kglobalshortcutinfo.cpp
  kglobalshortcutinfo_dbus.cpp
)
ecm_qt_declare_logging_category(kglobalaccel_SRCS HEADER kglobalaccel_debug.h IDENTIFIER KGLOBALACCEL_LOG CATEGORY_NAME kf5.kglobalaccel)

ecm_create_qm_loader(kglobalaccel_SRCS kglobalaccel5_qt)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf5_org.kde.KGlobalAccel.xml)

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kglobalaccel_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf5_org.kde.kglobalaccel.Component.xml)

add_library(KF5GlobalAccel ${kglobalaccel_SRCS})
generate_export_header(KF5GlobalAccel BASE_NAME KGlobalAccel)
add_library(KF5::GlobalAccel ALIAS KF5GlobalAccel)

target_include_directories(KF5GlobalAccel INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KGlobalAccel>")

target_link_libraries(KF5GlobalAccel
PUBLIC
    Qt5::DBus
    Qt5::Widgets
)

if(HAVE_X11)
    target_link_libraries(KF5GlobalAccel
    PRIVATE
        Qt5::X11Extras
    )
endif()

set_target_properties(KF5GlobalAccel PROPERTIES VERSION   ${KGLOBALACCEL_VERSION_STRING}
                                                SOVERSION ${KGLOBALACCEL_SOVERSION}
                                                EXPORT_NAME "GlobalAccel"
)

ecm_generate_headers(KGlobalAccel_HEADERS
  HEADER_NAMES
  KGlobalAccel
  KGlobalShortcutInfo

  REQUIRED_HEADERS KGlobalAccel_HEADERS
)

install(TARGETS KF5GlobalAccel EXPORT KF5GlobalAccelTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kglobalaccel_export.h
  ${KGlobalAccel_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KGlobalAccel COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5GlobalAccel_QCH
        NAME KGlobalAccel
        BASE_NAME KF5GlobalAccel
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KGlobalAccel_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Widgets_QCH
            Qt5DBus_QCH
        BLANK_MACROS
            KGLOBALACCEL_EXPORT
            KGLOBALACCEL_DEPRECATED
            KGLOBALACCEL_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KGlobalAccel LIB_NAME KF5GlobalAccel DEPS "dbus widgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KGlobalAccel)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

add_subdirectory(runtime)
