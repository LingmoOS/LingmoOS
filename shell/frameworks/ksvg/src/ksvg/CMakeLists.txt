

# Consumer's include dir which has to be explicitly used to make headers of this lib visible to documented includes
# Results in duplicate of prefix-dir & C++ namespace below, but part of different things, so by design:
# <visibility-layer>/<C++ namespace>/class header files
set(KSVG_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/KSvg")

add_library(KF6Svg)
add_library(KF6::Svg ALIAS KF6Svg)

qt_extract_metatypes(KF6Svg)

set_target_properties(KF6Svg PROPERTIES
    VERSION     ${KSVG_VERSION}
    SOVERSION   ${KSVG_SOVERSION}
    EXPORT_NAME Svg
)

target_sources(KF6Svg PRIVATE
    framesvg.cpp
    svg.cpp
    imageset.cpp
    private/imageset_p.cpp
)

ecm_qt_declare_logging_category(KF6Svg
    HEADER debug_p.h
    IDENTIFIER LOG_KSVG
    CATEGORY_NAME kf.svg
    DESCRIPTION "KSvg lib"
    EXPORT KSVG
)

ecm_generate_export_header(KF6Svg
    EXPORT_FILE_NAME ksvg/ksvg_export.h
    BASE_NAME KSvg
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

target_link_libraries(KF6Svg
PUBLIC
    Qt6::Gui
    KF6::ConfigCore
PRIVATE
    Qt6::Svg
    KF6::Archive
    KF6::CoreAddons
    KF6::GuiAddons #kimagecache
    KF6::ConfigCore
    KF6::ColorScheme
)

set(KSvg_BUILD_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/KSvg
)
target_include_directories(KF6Svg
    PUBLIC
        "$<BUILD_INTERFACE:${KSvg_BUILD_INCLUDE_DIRS}>"
    INTERFACE
        "$<INSTALL_INTERFACE:${KSVG_INSTALL_INCLUDEDIR}>"
)

########### install files ###############
ecm_generate_headers(KSvg_CamelCase_HEADERS
    HEADER_NAMES
        FrameSvg
        Svg
        ImageSet
    REQUIRED_HEADERS KSvg_namespaced_HEADERS
    PREFIX KSvg
)

install(
    FILES ${KSvg_namespaced_HEADERS}
          ${CMAKE_CURRENT_BINARY_DIR}/ksvg/ksvg_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KSvg/ksvg # C++ namespace
    COMPONENT Devel
)

install(
    FILES ${KSvg_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KSvg/KSvg # C++ namespace
    COMPONENT Devel
)

install(TARGETS KF6Svg EXPORT KF6SvgTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF6Svg_QCH
        NAME KSvg
        BASE_NAME KF6Svg
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KSvg_namespaced_HEADERS}
            ../../Mainpage.dox
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${KSvg_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KSVG_EXPORT
            KSVG_DEPRECATED
            KSVG_DEPRECATED_EXPORT
            "KSVG_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
