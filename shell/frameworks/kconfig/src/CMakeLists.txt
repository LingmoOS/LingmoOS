add_subdirectory(core)
if(TARGET Qt6::Gui)
    add_subdirectory(gui)
endif()
if(KCONFIG_USE_QML)
    add_subdirectory(qml)
endif()
add_subdirectory(kconfig_compiler)
add_subdirectory(kconf_update)
add_subdirectory(kreadconfig)

ecm_qt_install_logging_categories(
    EXPORT KCONFIG
    FILE kconfig.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Config_QCH
        NAME KConfig
        BASE_NAME KF6Config
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KConfigCore_APIDOX_SRCS}
            ${KConfigGui_APIDOX_SRCS}
            "${CMAKE_CURRENT_SOURCE_DIR}/kconfig_compiler/README.dox"
            "${CMAKE_SOURCE_DIR}/docs/options.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Xml_QCH
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${KConfigCore_APIDOX_INCLUDE_DIRS}
            ${KConfigGui_APIDOX_INCLUDE_DIRS}
        BLANK_MACROS
            KCONFIGCORE_EXPORT
            KCONFIGCORE_DEPRECATED_EXPORT
            KCONFIGCORE_DEPRECATED
            "KCONFIGCORE_DEPRECATED_VERSION(x, y, t)"
            "KCONFIGCORE_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KCONFIGCORE_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            KCONFIGGUI_EXPORT
            KCONFIGGUI_DEPRECATED_EXPORT
            KCONFIGGUI_DEPRECATED
            "KCONFIGGUI_DEPRECATED_VERSION(x, y, t)"
            "KCONFIGGUI_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KCONFIGGUI_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KCONFIGGUI_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
