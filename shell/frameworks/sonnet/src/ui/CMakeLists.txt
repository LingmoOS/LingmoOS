add_library(KF6SonnetUi)
add_library(KF6::SonnetUi ALIAS KF6SonnetUi)

set_target_properties(KF6SonnetUi PROPERTIES
    VERSION     ${SONNET_VERSION}
    SOVERSION   ${SONNET_SOVERSION}
    EXPORT_NAME SonnetUi
)

ecm_create_qm_loader(KF6SonnetUi sonnet6_qt)

qt_wrap_ui(sonnetui_ui_SRCS
    configui.ui
    sonnetui.ui
)

target_sources(KF6SonnetUi PRIVATE
    ${sonnetui_ui_SRCS}
    configdialog.cpp
    configview.cpp
    configwidget.cpp
    dialog.cpp
    dictionarycombobox.cpp
    highlighter.cpp
    spellcheckdecorator.cpp
)

ecm_qt_declare_logging_category(KF6SonnetUi
    HEADER ui_debug.h
    IDENTIFIER SONNET_LOG_UI
    CATEGORY_NAME kf.sonnet.ui
    OLD_CATEGORY_NAMES sonnet.ui
    DESCRIPTION "Sonnet UI"
    EXPORT SONNET
)

ecm_generate_headers(SonnetUi_CamelCase_HEADERS
  HEADER_NAMES
    Dialog
    Highlighter
    ConfigDialog
    ConfigView
    ConfigWidget
    DictionaryComboBox
    SpellCheckDecorator
  PREFIX Sonnet
  REQUIRED_HEADERS SonnetUi_HEADERS
)

ecm_generate_export_header(KF6SonnetUi
    BASE_NAME SonnetUi
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME Sonnet
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6SonnetUi
    PUBLIC Qt6::Widgets
    PRIVATE KF6::SonnetCore
)

target_include_directories(KF6SonnetUi
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/SonnetUi>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Sonnet>"
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>" # version header
)

install(TARGETS KF6SonnetUi EXPORT KF6SonnetTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${SonnetUi_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/SonnetUi/Sonnet COMPONENT Devel)

install(FILES
   ${SonnetUi_HEADERS}
   ${CMAKE_CURRENT_BINARY_DIR}/sonnetui_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/SonnetUi/sonnet COMPONENT Devel)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

if (BUILD_QCH)
    ecm_add_qch(
        KF6SonnetUi_QCH
        NAME SonnetUi
        BASE_NAME KF6SonnetUi
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${SonnetUi_HEADERS}
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            SONNETUI_EXPORT
            SONNETUI_DEPRECATED_EXPORT
            SONNETUI_DEPRECATED
            "SONNETUI_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

