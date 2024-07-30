# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kcmutils_logging_STATIC STATIC)
set_target_properties(kcmutils_logging_STATIC PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kcmutils_logging_STATIC PRIVATE Qt6::Core)
ecm_qt_declare_logging_category(kcmutils_logging_STATIC
    HEADER kcmutils_debug.h
    IDENTIFIER KCMUTILS_LOG
    CATEGORY_NAME kf.kcmutils
    DESCRIPTION "KCMUtils"
    EXPORT KCMUTILS
)


add_subdirectory(core)
add_subdirectory(qml)

########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduleloader.cpp
    kcmoduleloader.h
    kcmoduleqml.cpp
    kcmoduleqml_p.h
    kcmultidialog.cpp
    kcmultidialog.h
    kcmultidialog_p.h
    kpluginwidget.cpp
    kcmodule.cpp
    kcmodule.h
)

add_library(KF6KCMUtils ${kcmutils_LIB_SRCS})

set_target_properties(KF6KCMUtils PROPERTIES
    VERSION     ${KCMUTILS_VERSION}
    SOVERSION   ${KCMUTILS_SOVERSION}
    EXPORT_NAME KCMUtils
)

if (WITH_KAUTH)
    set(WITH_KAUTH_DEFINE_VALUE 1)
else()
    set(WITH_KAUTH_DEFINE_VALUE 0)
endif()
set(define_with_kauth_code "#define KCMUTILS_WITH_KAUTH ${WITH_KAUTH_DEFINE_VALUE}\n")

ecm_generate_export_header(KF6KCMUtils
    BASE_NAME KCMUtils
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    CUSTOM_CONTENT_FROM_VARIABLE define_with_kauth_code
)
target_include_directories(KF6KCMUtils INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils>")

target_link_libraries(KF6KCMUtils
  PUBLIC
    Qt6::Widgets
    KF6::CoreAddons # KPluginMetaData
    KF6::ConfigWidgets  # KPageDialog
    KF6KCMUtilsQuick      # QML KCM class
  PRIVATE
    kcmutils_proxy_model
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickWidgets
    KF6::GuiAddons # KIconUtils
    KF6::I18n
    KF6::ItemViews       # KWidgetItemDelegate
    KF6::XmlGui          # KAboutApplicationDialog
    kcmutils_logging_STATIC
)

ecm_generate_headers(KCMUtils_HEADERS
  HEADER_NAMES
  KCModuleLoader
  KCMultiDialog
  KPluginWidget
  KCModule

  REQUIRED_HEADERS KCMUtils_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
  ${KCMUtils_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils COMPONENT Devel
)

install(TARGETS KF6KCMUtils EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF6KCMUtils_QCH
        NAME KCMUtils
        BASE_NAME KF6KCMUtils
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCMUtils_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Widgets_QCH
            KF6ConfigWidgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCMUTILS_EXPORT
            KCMUTILS_DEPRECATED
            KCMUTILS_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT KCMUTILS
    FILE kcmutils.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

add_subdirectory(kcmshell)
