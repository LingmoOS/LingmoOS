add_library(KF6IconWidgets)
add_library(KF6::IconWidgets ALIAS KF6IconWidgets)

set_target_properties(KF6IconWidgets PROPERTIES
    VERSION     ${KICONTHEMES_VERSION}
    SOVERSION   ${KICONTHEMES_SOVERSION}
    EXPORT_NAME IconWidgets
)

target_sources(KF6IconWidgets PRIVATE
    kiconbutton.cpp
    kiconbutton.h
    kicondialog.cpp
    kicondialog.h
    kicondialogmodel_p.h
    kicondialog_p.h
    kpixmapsequenceloader.h
    kpixmapsequenceloader.cpp
    )

ki18n_wrap_ui(KF6IconWidgets kicondialog.ui)

ecm_generate_export_header(KF6IconWidgets
    BASE_NAME KIconWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KIconThemes
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6IconWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KIconWidgets>")

target_link_libraries(KF6IconWidgets
  PUBLIC
    Qt6::Widgets
    KF6::IconThemes
  PRIVATE
    Qt6::GuiPrivate
    Qt6::Svg
    KF6::I18n          # for i18n in KIconDialog
    KF6::ConfigWidgets # for KStandardAction
    KF6::WidgetsAddons # for KPixmapSequence
)

ecm_generate_headers(KIconWidgets_HEADERS
  HEADER_NAMES
  KIconButton
  KIconDialog
  KPixmapSequenceLoader

  REQUIRED_HEADERS KIconWidgets_HEADERS
)

install(TARGETS KF6IconWidgets EXPORT KF6IconThemesTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kiconwidgets_export.h
  ${KIconWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIconWidgets COMPONENT Devel
)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF6IconWidgets_QCH
        NAME KIconWidgets
        BASE_NAME KF6IconWidgets
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIconWidgets_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KICONWIDGETS_EXPORT
            KICONWIDGETS_DEPRECATED
            KICONWIDGETS_DEPRECATED_EXPORT
            "KICONWIDGETS_DEPRECATED_VERSION(x, y, t)"
            "KICONWIDGETS_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KICONWIDGETS_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KICONWIDGETS_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

