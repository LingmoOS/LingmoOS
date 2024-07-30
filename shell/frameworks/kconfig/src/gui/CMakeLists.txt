add_library(KF6ConfigGui)
add_library(KF6::ConfigGui ALIAS KF6ConfigGui)

qt_extract_metatypes(KF6ConfigGui)

set_target_properties(KF6ConfigGui PROPERTIES
    VERSION     ${KCONFIG_VERSION}
    SOVERSION   ${KCONFIG_SOVERSION}
    EXPORT_NAME ConfigGui
)

ecm_create_qm_loader(KF6ConfigGui kconfig6_qt)

target_sources(KF6ConfigGui PRIVATE
    kconfiggui.cpp
    kconfiggroupgui.cpp
    kconfigloader.cpp
    kconfigskeleton.cpp
    kstandardshortcut.cpp
    kstandardshortcutwatcher.cpp
    kwindowconfig.cpp
    kwindowstatesaver.cpp
    kstandardactions.cpp
)

ecm_qt_declare_logging_category(KF6ConfigGui
    HEADER kconfig_gui_log_settings.h
    IDENTIFIER KCONFIG_GUI_LOG
    CATEGORY_NAME kf.config.gui
    DESCRIPTION "KConfig Gui"
    EXPORT KCONFIG
)

ecm_generate_export_header(KF6ConfigGui
    BASE_NAME KConfigGui
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KConfig
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6ConfigGui
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui>")

target_link_libraries(KF6ConfigGui PUBLIC Qt6::Gui KF6::ConfigCore)

ecm_generate_headers(KConfigGui_HEADERS
  HEADER_NAMES
  KConfigGui
  KConfigLoader
  KConfigSkeleton
  KStandardShortcut
  KStandardShortcutWatcher
  KWindowConfig
  KWindowStateSaver
  KStandardActions

  REQUIRED_HEADERS KConfigGui_HEADERS
)

if(NOT BUILD_SHARED_LIBS)
    target_sources(KF6ConfigGui PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kconfigguistaticinitializer.cpp>
        $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui/kconfigguistaticinitializer.cpp>
    )
endif()

install(TARGETS KF6ConfigGui EXPORT KF6ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
  kstandardactions_p.h
  ${KConfigGui_HEADERS}
  kconfigguistaticinitializer.cpp
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KConfigGui_APIDOX_SRCS ${KConfigGui_HEADERS} PARENT_SCOPE)
set(KConfigGui_APIDOX_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

