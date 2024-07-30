# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KF6KCMUtilsCore)

qt_extract_metatypes(KF6KCMUtilsCore)

set_target_properties(KF6KCMUtilsCore PROPERTIES
    VERSION     ${KCMUTILS_VERSION}
    SOVERSION   ${KCMUTILS_SOVERSION}
    EXPORT_NAME KCMUtilsCore
)

ecm_qt_declare_logging_category(KF6KCMUtilsCore
    HEADER kcmutilscore_debug.h
    IDENTIFIER KCMUTILS_CORE_LOG
    CATEGORY_NAME kf.kcmutils.core
    DESCRIPTION "KCMUtilsCore"
    EXPORT KCMUTILS_CORE
)
ecm_generate_export_header(KF6KCMUtilsCore
    BASE_NAME KCMUtilsCore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KCMUtils
    DEPRECATED_BASE_VERSION 0
)
target_include_directories(KF6KCMUtilsCore PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # module version header
)
target_include_directories(KF6KCMUtilsCore INTERFACE
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsCore>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils>" # module version header
)

target_sources(KF6KCMUtilsCore PRIVATE
    kpluginmodel.cpp
    kabstractconfigmodule.cpp
    kcmoduledata.cpp
    kcmoduledata.h
)
target_link_libraries(KF6KCMUtilsCore
  PUBLIC
    KF6::CoreAddons
  PRIVATE
    KF6::ConfigCore
    KF6::ItemViews
)

ecm_generate_headers(KCMUtilsCore_HEADERS
  HEADER_NAMES
  KPluginModel
  KAbstractConfigModule
  KCModuleData

  REQUIRED_HEADERS KCMUtilsCore_HEADERS
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcmutilscore_export.h
  ${KCMUtilsCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtilsCore COMPONENT Devel
)

install(TARGETS KF6KCMUtilsCore EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

add_library(kcmutils_proxy_model STATIC kpluginproxymodel.cpp)
qt_extract_metatypes(kcmutils_proxy_model)
# Needed to link this static lib to shared libs
set_property(TARGET kcmutils_proxy_model PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kcmutils_proxy_model PUBLIC KF6KCMUtilsCore KF6::ItemViews)

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS kcmutils_proxy_model EXPORT KF6KCMUtilsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()
