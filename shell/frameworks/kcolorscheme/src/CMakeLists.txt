# SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6ColorScheme)
add_library(KF6::ColorScheme ALIAS KF6ColorScheme)

set_target_properties(KF6ColorScheme PROPERTIES
    VERSION     ${KCOLORSCHEME_VERSION}
    SOVERSION   ${KCOLORSCHEME_SOVERSION}
    EXPORT_NAME ColorScheme
)

target_sources(KF6ColorScheme PRIVATE
  kcolorscheme.cpp
  kcolorschememanager.cpp
  kcolorschememodel.cpp
  kstatefulbrush.cpp
)

qt_add_resources(KF6ColorScheme "color_schemes"
    PREFIX "/org.kde.kcolorscheme"
    FILES
        color-schemes/BreezeDark.colors
        color-schemes/BreezeLight.colors
)

ecm_qt_declare_logging_category(KF6ColorScheme
    HEADER kcolorscheme_debug.h
    IDENTIFIER KCOLORSCHEME
    CATEGORY_NAME kf.colorscheme
    DESCRIPTION "KColorScheme"
    EXPORT KCOLORSCHEME
)

ecm_generate_export_header(KF6ColorScheme
    BASE_NAME KColorScheme
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6ColorScheme INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KColorScheme>")

target_link_libraries(KF6ColorScheme
  PUBLIC
    KF6::ConfigCore
    Qt6::Gui
  PRIVATE
    KF6::ConfigGui # ### this one is not directly used but has to be loaded in order for QColor I/O from KConfig to work!
    KF6::GuiAddons # KColorScheme uses KColorUtils
    KF6::I18n # For action and widget texts
)

ecm_generate_headers(KColorScheme_HEADERS
  HEADER_NAMES
  KColorScheme
  KColorSchemeManager
  KColorSchemeModel
  KStatefulBrush

  REQUIRED_HEADERS KColorScheme_HEADERS
)

install(TARGETS KF6ColorScheme EXPORT KF6ColorSchemeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcolorscheme_export.h
  ${KColorScheme_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KColorScheme COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KCOLORSCHEME
    FILE kcolorscheme.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)


if(BUILD_QCH)
    ecm_add_qch(
        KF6ColorScheme_QCH
        NAME KColorScheme
        BASE_NAME KF6ColorScheme
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KColorScheme_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            KF6GuiAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCOLORSCHEME_EXPORT
            KCOLORSCHEME_DEPRECATED
            KCOLORSCHEME_DEPRECATED_EXPORT
            "KCOLORSCHEME_DEPRECATED_VERSION(x, y, t)"
            "KCOLORSCHEME_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KCOLORSCHEME_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KCOLORSCHEME_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        PREDEFINED_MACROS
            "KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(x, y)=1"
            "KCOLORSCHEME_BUILD_DEPRECATED_SINCE(x, y)=1"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

