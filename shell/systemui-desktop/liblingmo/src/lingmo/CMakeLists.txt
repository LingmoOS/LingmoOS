add_subdirectory(packagestructure)

# Consumer's include dir which has to be explicitly used to make headers of this lib visible to documented includes
# Results in duplicate of prefix-dir & C++ namespace below, but part of different things, so by design:
# <visibility-layer>/<C++ namespace>/class header files
set(LINGMO_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/Lingmo")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-lingmo.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-lingmo.h)

add_library(Lingmo)
add_library(Lingmo::Lingmo ALIAS Lingmo)

qt_extract_metatypes(Lingmo)

set_target_properties(Lingmo PROPERTIES
    VERSION     ${LINGMO_VERSION}
    SOVERSION   ${LINGMO_SOVERSION}
    EXPORT_NAME Lingmo
)

target_sources(Lingmo PRIVATE
#global
    lingmo.cpp
    pluginloader.cpp

#applets,containments,corona
    applet.cpp
    containment.cpp
    containmentactions.cpp
    corona.cpp
    private/applet_p.cpp
    private/containment_p.cpp

#graphics
    theme.cpp
    private/theme_p.cpp
)

if(HAVE_X11)
    target_sources(Lingmo PRIVATE private/effectwatcher.cpp)
endif()

kconfig_add_kcfg_files(Lingmo data/kconfigxt/liblingmo-theme-global.kcfgc)

ecm_qt_declare_logging_category(Lingmo
    HEADER debug_p.h
    IDENTIFIER LOG_LINGMO
    CATEGORY_NAME kf.lingmo.core
    OLD_CATEGORY_NAMES org.kde.lingmo
    DESCRIPTION "Lingmo Core lib"
    EXPORT LINGMO
)

ecm_generate_export_header(Lingmo
    EXPORT_FILE_NAME lingmo/lingmo_export.h
    BASE_NAME Lingmo
    GROUP_BASE_NAME KF
    VERSION ${KF6_MIN_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

if(HAVE_X11)
    set(LINGMO_EXTRA_LIBS ${LINGMO_EXTRA_LIBS} XCB::XCB)
endif()

target_link_libraries(Lingmo
PUBLIC
    Qt6::Gui
    KF6::ConfigCore
    KF6::CoreAddons
    Qt6::Qml
PRIVATE
    Qt6::Quick
    KF6::ConfigGui
    KF6::ColorScheme
    KF6::ConfigQml
    KF6::Archive
    KF6::GuiAddons #kimagecache
    KF6::I18n
    KF6::WindowSystem #compositingActive
    KF6::GlobalAccel #Applet::setGlobalShortcut
    KF6::Notifications
    KF6::IconThemes
    Lingmo::Activities
    KF6::Svg
    ${LINGMO_EXTRA_LIBS}
)

target_link_libraries(Lingmo PRIVATE KF6::Package)

set(Lingmo_BUILD_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/Lingmo
)
target_include_directories(Lingmo
    PUBLIC
        "$<BUILD_INTERFACE:${Lingmo_BUILD_INCLUDE_DIRS}>"
    INTERFACE
        "$<INSTALL_INTERFACE:${LINGMO_INSTALL_INCLUDEDIR}>"
)

########### install files ###############
ecm_generate_headers(Lingmo_CamelCase_HEADERS
    HEADER_NAMES
        Applet
        Containment
        ContainmentActions
        Corona
        PluginLoader
        Theme
        Lingmo
    REQUIRED_HEADERS Lingmo_HEADERS
    PREFIX Lingmo
)

install(FILES
        ${Lingmo_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo/lingmo_export.h
    DESTINATION ${LINGMO_INSTALL_INCLUDEDIR}/lingmo # C++ namespace
    COMPONENT Devel
)

install(
    FILES ${Lingmo_CamelCase_HEADERS}
    DESTINATION ${LINGMO_INSTALL_INCLUDEDIR}/Lingmo # C++ namespace
    COMPONENT Devel
)

install(TARGETS Lingmo EXPORT LingmoTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        Lingmo_QCH
        NAME Lingmo
        BASE_NAME Lingmo
        VERSION ${LINGMO_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Lingmo_HEADERS}
            Mainpage.dox
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
            KF6Service_QCH
            KF6Package_QCH
        INCLUDE_DIRS
            ${Lingmo_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            LINGMO_EXPORT
            LINGMO_DEPRECATED
            LINGMO_DEPRECATED_EXPORT
            "LINGMO_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
