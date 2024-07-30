# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

# Library
add_library(KF6KCMUtilsQuick
    kquickconfigmodule.cpp
    kquickmanagedconfigmodule.cpp
    sharedqmlengine.cpp
    kquickconfigmoduleloader.cpp
)

qt_extract_metatypes(KF6KCMUtilsQuick)

ecm_generate_export_header(KF6KCMUtilsQuick
    BASE_NAME KCMUtilsQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
)
target_include_directories(KF6KCMUtilsQuick INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQuick>")

target_link_libraries(KF6KCMUtilsQuick
  PUBLIC
    KF6::CoreAddons
    KF6::ConfigCore
    Qt6::Qml
    KF6KCMUtilsCore
  PRIVATE
    KF6::I18n
    KF6::ConfigGui
    Qt6::Quick
    kcmutils_logging_STATIC
)

set_target_properties(KF6KCMUtilsQuick PROPERTIES
                      VERSION ${KCMUTILS_VERSION}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtilsQuick)
ecm_generate_headers(KCMUtilsQuick_HEADERS
  HEADER_NAMES
  KQuickConfigModule
  KQuickManagedConfigModule
  KQuickConfigModuleLoader

  REQUIRED_HEADERS KCMUtilsQuick_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutilsquick_export.h
  ${KCMUtilsQuick_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsQuick COMPONENT Devel
)

install(TARGETS KF6KCMUtilsQuick EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

# QML plugin
ecm_add_qml_module(kcmutilsqmlplugin URI "org.kde.kcmutils" VERSION 1.0 DEPENDENCIES QtQuick org.kde.kcmutils.private org.kde.config GENERATE_PLUGIN_SOURCE)
target_sources(kcmutilsqmlplugin
  PRIVATE
    settingstateproxy.cpp
    kcmlauncher.cpp
    types.h
)
target_link_libraries(kcmutilsqmlplugin
  PRIVATE
    KF6::KIOGui
    Qt6::Qml
    Qt6::Quick
    kcmutils_proxy_model
    kcmutils_logging_STATIC
    KF6KCMUtilsQuick
)

ecm_target_qml_sources(kcmutilsqmlplugin SOURCES
    components/PluginDelegate.qml
    components/PluginSelector.qml
    components/AbstractKCM.qml
    components/ContextualHelpButton.qml
    components/GridDelegate.qml
    components/GridView.qml
    components/GridViewKCM.qml
    components/ScrollView.qml
    components/ScrollViewKCM.qml
    components/SettingHighlighter.qml
    components/SettingStateBinding.qml
    components/SimpleKCM.qml
)
ecm_target_qml_sources(kcmutilsqmlplugin PATH private SOURCES
    components/private/AboutPlugin.qml
    components/private/GridDelegateMenu.qml
    components/private/GridViewInternal.qml
)
ecm_finalize_qml_module(kcmutilsqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})

ecm_add_qml_module(kcmutilsprivateqmlplugin URI "org.kde.kcmutils.private" DEPENDENCIES QtCore QtQuick GENERATE_PLUGIN_SOURCE)
target_sources(kcmutilsprivateqmlplugin PRIVATE private_types.h settinghighlighterprivate.cpp)
target_link_libraries(kcmutilsprivateqmlplugin PRIVATE Qt6::Quick kcmutils_proxy_model)
ecm_finalize_qml_module(kcmutilsprivateqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})

